#include "address.h"
#include "memory.h"
#include "cpio.h"
#include "uart.h"
#include "math.h"
#include "type.h"
#include "interrupt.h"

uint64 _stack_size = 0x10000ULL;
page_t* usedPage = NULL;

void* simple_malloc(unsigned int size) {
    char* r = heap + 0x10;
    if(size < 0x18)
        size = 0x18;  // minimum size 0x20 //like ptmalloc
    size = size + 0x7;
    size = 0x10 + size - size % 0x10;
    *(unsigned int*)(r - 0x8) = size;
    heap += size;

    if(heap > heap_end) {
        raiseError("Run out of heap\n");
    }

    return r;
}

char* memcpy (void *dest, const void *src, unsigned long long len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}

pageHeader_t pagesAtOrder[MAX_ORDER];
uint64 usedAddress[MAX_PAGES];

void initMemoryPages() {
    for(int order = 0; order < (MAX_ORDER - 1); order++) {
        pagesAtOrder[order].firstFree = createPage(order, PAGE_BEGIN_ADDRESS, EMPTY);
        pagesAtOrder[order].firstEmpty = pagesAtOrder[order].firstFree;

        /* 
        The memory used by paging is allocated by simple_malloc which isn't free-able.
        To avoid memory leaking, we allocate all the memory used by paging at once.
        */
        page_t* page = pagesAtOrder[order].firstFree;
        int pageNum = getPageNumAtOrder(order);
        // uart_puts("There are "); uart_num(pageNum); uart_puts(" pages in order: "); uart_num(order); uart_newline();

        for(int i = 1; i < pageNum; i++) {
            page->nextPage = createPage(order, PAGE_BEGIN_ADDRESS + i * getPageSizeAtOrder(order), EMPTY);
            page->nextPage->prevPage = page; 
            page = page->nextPage;
        }
    }

    // uart_puts("There are 1 page in max order: "); uart_num(MAX_ORDER - 1); uart_newline();
    pagesAtOrder[MAX_ORDER - 1].firstFree = createPage(MAX_ORDER - 1, PAGE_BEGIN_ADDRESS, FREE);
    pagesAtOrder[MAX_ORDER - 1].firstEmpty = NULL;
    
    init_reserve();
    for(int i = 0; i < MAX_PAGES; i++) {
        usedAddress[i] = NULL;
    }

    // loggingAllPageState();
}

page_t* createPage(uint32 order, uint64 address, int state) {
    page_t* newPage = (page_t*)simple_malloc(sizeof(page_t));
    newPage->order = order;
    newPage->address = address;
    newPage->state = state;
    newPage->prevPage = NULL;
    newPage->nextPage = NULL;
    newPage->buddyPage = NULL;
    newPage->parentPage = NULL;
    newPage->appendPage = NULL;
    newPage->pool = (memoryPool_t*)simple_malloc(sizeof(memoryPool_t));
    return newPage;
}

void initPool(memoryPool_t* pool, uint32 maxSize, uint32 size, uint64 address) {
    pool->address = address;
    pool->size = size; 
    pool->stateBitMap = ~(0x00ULL);
    int num = maxSize / size;
    pool->mask = pool->stateBitMap >> (64 - num);
}

void* malloc(uint32 size) {
    lock_interrupt();
    uint32 order = sizeToSmallestOrder(size);
    size = exp2(order); 
    order = log2(size / (MINI_PAGE_SIZE * KB));

    // uart_puts("size  = "); uart_num(size); uart_newline();
    // uart_puts("order  = "); uart_num(order); uart_newline();

    if(usedPage == NULL) {
        // uart_puts("Init used page\n");
        usedPage = getFreePageAtOrder(order);
        initPool(usedPage->pool, getPageSizeAtOrder(order), size, usedPage->address);
    }
    else if(isAllFull(usedPage, size)) {
        // uart_puts("Allocate new page\n");
        page_t* newPage = getFreePageAtOrder(order);
        initPool(newPage->pool, getPageSizeAtOrder(order), size, newPage->address);
        appendNewPage(usedPage, newPage);
    }

    // showAppendPage(usedPage);
    void *address = allocateMemoryFromPage(usedPage, size);
    unlock_interrupt();

    return address;
}

uint32 sizeToSmallestOrder(uint32 size) {
    uint32 order = log2(size);
    if(size > exp2(order)) {
        order++;
    }

    return order;
}

void appendNewPage(page_t* parentPage, page_t* newPage) {
    page_t* page = parentPage;
    while(page->appendPage != NULL) {
        page = page->appendPage;
    }

    newPage->appendPage = NULL;
    page->appendPage = newPage;
}

void showAppendPage(page_t *parentPage) {
    page_t* page = parentPage;
    while(page != NULL) {
        loggingPage(page);
        page = page->appendPage;
    }
}

bool isAllFull(page_t* page, uint32 size) {
    while(page != NULL) {
        if(isFull(page, size)) {
            page = page->appendPage;
        }
        else {
            return false;
        }
    }

    return true;
}

bool isFull(page_t* page, uint32 size) {
    return isPoolFull(page->pool, size);
}

bool isPoolFull(memoryPool_t* pool, uint32 size) {
    return size > pool->size || (pool->stateBitMap & pool->mask) == 0ULL;
}

void* allocateMemoryFromPage(page_t* page, uint32 size) {
    page_t* allocatedPage = NULL;

    // uart_puts("Allocate page for memory\n");
    while(page != NULL) {
        if(!isFull(page, size)) {
            allocatedPage = page;
            break;
        }

        page = page->appendPage;
    }

    if(allocatedPage == NULL) {
        raiseError("Can't allocate any memory from pages\n");
    }

    // uart_puts("Allocate pool for memory\n");
    memoryPool_t* allocatedPool = allocatedPage->pool;

    if(allocatedPool == NULL) {
        raiseError("Can't allocate any memory from pools (NULL)\n");
    }
    else if(isPoolFull(allocatedPool, size)) {
        raiseError("Can't allocate any memory from pools (FULL)\n");
    }
    
    uint32 shift = log2(allocatedPool->stateBitMap & allocatedPool->mask);
    allocatedPool->stateBitMap &= ~(1ULL << shift);
    // loggingPool(allocatedPool);

    // uart_puts("Mark as used memory, state = ");
    // uart_hex(allocatedPool->stateBitMap >> 32); uart_hex(allocatedPool->stateBitMap);
    // uart_newline();
    // uart_puts("AllocatedPage at "); uart_hex(allocatedPage->address); uart_newline();
    registerUsedAddress(allocatedPage);
    return (void*)(allocatedPool->address + shift * allocatedPool->size);   
}

void registerUsedAddress(page_t* page) {
    uint32 idx = addressToBlockIdx(page->address);
    uint32 numCoverdPage = exp2(page->order);

    for(int i = 0; i < numCoverdPage; i++) {
        usedAddress[i + idx] = page; 
    }
}

uint32 addressToBlockIdx(uint64 address) {
    return (address - PAGE_BEGIN_ADDRESS) / (MINI_PAGE_SIZE * KB);
}

page_t* getFreePageAtOrder(uint32 order) {
    if(order >= MAX_ORDER) {
        uart_num(order); uart_puts(" is higher than max order: "); uart_num(MAX_ORDER); uart_newline();
        raiseError("Run out of memory\n");
    }

    if(hasNoFreePageAtOrder(order)) {
        // uart_puts("Has no free page at order: "); uart_num(order); uart_puts(", get page from higher order\n");
        page_t* pageAtHigherOrder = getFreePageAtOrder(order + 1);
        // uart_puts("Divide page from order: "); uart_num(order + 1); uart_puts(" to order: "); uart_num(order); uart_newline();
        dividePageToLowerOrder(pageAtHigherOrder);
    }
    
    // uart_puts("Allocate page at order: "); uart_num(order); uart_newline();
    page_t* page = pagesAtOrder[order].firstFree;   
    // loggingPage(page); 
    page->appendPage = NULL;
    page->state = BUSY;
    
    if(isNotPageTail(pagesAtOrder[order].firstFree)) {
        pagesAtOrder[order].firstFree = pagesAtOrder[order].firstFree->nextPage;
    }
    // else {
    //     uart_puts("Full use at order: "); uart_num(order); uart_newline();
    // }
    
    return page;
}

void loggingPage(page_t* page) {
    uart_puts("Page ("); uart_hex(page); uart_puts(") ");
    uart_puts("Order: "); uart_num(page->order); uart_puts(" ");
    uart_puts("Address: "); uart_hex(page->address); uart_puts(" ~ "); 
        uart_hex(page->address + getPageSizeAtOrder(page->order)); uart_puts(" ");
    uart_puts("State: "); uart_num(page->state); uart_newline();
    // uart_puts("Previous page: "); uart_hex(page->prevPage); uart_puts(" ");
    // uart_puts("Next page: "); uart_hex(page->nextPage); uart_newline();
    // uart_puts("Buddy page: "); uart_hex(page->buddyPage); uart_puts(" ");
    // uart_puts("Parent Page: "); uart_hex(page->parentPage); uart_newline();

    // memoryPool_t* pool = page->headPool;
    // for(int i = 0; i < NUM_POOL; i++) {
    //     loggingPool(pool);
    //     pool = pool->nextPool;
    // }
}

void loggingPool(memoryPool_t* pool) {
    uart_puts("Pool ("); uart_hex(pool); uart_puts(") ");
    uart_puts("Address: "); uart_hex(pool->address); uart_puts(" ");
    uart_puts("Size: "); uart_num(pool->size); uart_puts(" ");
    uart_puts("Mask: "); uart_hex(pool->mask>>32); uart_hex(pool->mask); uart_newline();

    uint64 state = pool->stateBitMap & pool->mask;
    uart_puts("State: "); uart_hex(state>>32); uart_hex(state); uart_newline();
}

bool hasFreePageAtOrder(uint32 order) {
    return pagesAtOrder[order].firstFree->state == FREE;
}

bool hasNoFreePageAtOrder(uint32 order) {
    return pagesAtOrder[order].firstFree->state != FREE;
}

void dividePageToLowerOrder(page_t* page) {
    uint32 order = page->order;
    uint64 address = page->address;
    page_t* topHalfPage = addPageAtOrder(order - 1, address, page);
    page_t* bottomHalfPage = addPageAtOrder(order - 1, address + getPageSizeAtOrder(order - 1), page);
    bindAsBuddy(topHalfPage, bottomHalfPage);
}

page_t* addPageAtOrder(uint32 order, uint64 address, page_t* parentPage) {
    page_t* page = pagesAtOrder[order].firstEmpty;
    page->address = address;
    page->state = FREE;
    page->parentPage = parentPage;
    if(isNotPageTail(pagesAtOrder[order].firstEmpty)) {
        pagesAtOrder[order].firstEmpty = pagesAtOrder[order].firstEmpty->nextPage;
    }
    // else {
    //     uart_puts("No any empty page at order: "); uart_num(order); uart_newline();
    // }
    return page;
}

uint64 getPageSizeAtOrder(uint32 order) {
    uint64 num = exp2(order);
    return num * MINI_PAGE_SIZE * KB;
}

uint64 getPageNumAtOrder(uint32 order) {
    return exp2(MAX_ORDER - 1 - order);
}

void bindAsBuddy(page_t* page1, page_t* page2) {
    page1->buddyPage = page2;
    page2->buddyPage = page1;
}

void freePage(page_t* page) {
    if(page->state != BUSY) {
        uart_puts("Not free-able\n");
        return;
    }

    uint32 order = page->order;
    page->state = FREE;

    // uart_puts("Free page at order: "); uart_num(order); uart_newline();

    if(page->order == (MAX_ORDER - 1)) {
        return;
    }

    if(!isFirstEmpty(page)) {    
        if(isNotPageHead(page)) {
            // uart_puts("Relink page at order: "); uart_num(order); uart_newline();
            page->prevPage->nextPage = page->nextPage;
        }
        
        if(pagesAtOrder[order].firstEmpty->state == EMPTY) {
            page->nextPage->prevPage = page->prevPage;
            page->nextPage = pagesAtOrder[order].firstEmpty;
            page->prevPage = pagesAtOrder[order].firstEmpty->prevPage;
            pagesAtOrder[order].firstEmpty->prevPage->nextPage = page;
            pagesAtOrder[order].firstEmpty->prevPage = page;
        }
        else {
            // uart_puts("Full use case at order: "); uart_num(order); uart_newline();
            page->nextPage->prevPage = page->prevPage;
            page->prevPage = pagesAtOrder[order].firstEmpty;
            page->nextPage = pagesAtOrder[order].firstEmpty->nextPage;
            pagesAtOrder[order].firstEmpty->nextPage->prevPage = page;
            pagesAtOrder[order].firstEmpty->nextPage = page;
            pagesAtOrder[order].firstEmpty = page;
        }
    }

    if(hasNoFreePageAtOrder(order)) {
        // uart_puts("Assign as first free page at order: "); uart_num(order); uart_newline();
        pagesAtOrder[order].firstFree = page;
    }

    if(page->buddyPage->state == FREE) {
        // uart_puts("Buddy is free too, merge pages from order: "); uart_num(order); uart_puts(" to order: "); uart_num(order + 1); uart_newline();
        mergePagesToHigherOrder(page);
    }
}

bool isNotPageHead(page_t* page) {
    return page->prevPage != NULL;
}

bool isNotPageTail(page_t* page) {
    return page->nextPage != NULL;
}

bool isFirstFree(page_t* page) {
    return page == pagesAtOrder[page->order].firstFree;
}

bool isFirstEmpty(page_t* page) {
    return page == pagesAtOrder[page->order].firstEmpty;
}

void mergePagesToHigherOrder(page_t* page) {
    moveToTail(page);
    moveToTail(page->buddyPage);
    freePage(page->parentPage);
}

void moveToTail(page_t* page) {
    uint32 order = page->order;
    page->state = EMPTY;

    if(!isFirstEmpty(page)) {    
        if(isFirstFree(page)) {
            pagesAtOrder[order].firstFree = page->nextPage;
        }

        if(isNotPageHead(page)) {
            page->prevPage->nextPage = page->nextPage;
        }

        page->nextPage->prevPage = page->prevPage;
        page->prevPage = pagesAtOrder[order].firstEmpty;
        page->nextPage = pagesAtOrder[order].firstEmpty->nextPage;
        pagesAtOrder[order].firstEmpty->nextPage->prevPage = page;
        pagesAtOrder[order].firstEmpty->nextPage = page;
    }
}

void loggingAllPageState() {
    for(int order = 0; order < MAX_ORDER; order++) {
        page_t* page = pagesAtOrder[order].firstFree;
        while(isNotPageHead(page)) {
            page = page->prevPage;
        }

        while(page != NULL) { 
            switch(page->state) {
                case FREE: 
                    uart_putc('F');
                break;
                case BUSY: 
                    uart_putc('B');
                break;
                case EMPTY: 
                    uart_putc('X');
                break;
                default:
                    uart_putc('?');
                break;
            }

            uart_putc(' ');
            page = page->nextPage;
        }

        uart_newline();   
    } 
}

void free(uint64 address) {
    lock_interrupt();
    page_t* page = usedAddress[addressToBlockIdx(address)];

    if(page == NULL) {
        uart_puts("Not free-able address (page)\n");
        return;
    }

    memoryPool_t* pool = page->pool;

    // uart_puts("Original state = ");
    // uart_hex(pool->stateBitMap >> 32); uart_hex(pool->stateBitMap);
    // uart_newline();

    uint32 shift = (address - pool->address) / pool->size;
    pool->stateBitMap |= 1ULL << shift;

    // uart_puts("Mark as free memory, state = ");
    // uart_hex(pool->stateBitMap >> 32); uart_hex(pool->stateBitMap);
    // uart_newline();

    page_t* page_pointer = usedPage;
    if(pool->stateBitMap == ~(0ULL)) {
        // uart_puts("All pools are free, free page: ");
        // uart_hex(page);
        // uart_newline();
        if(page == usedPage) {
            usedPage = usedPage->appendPage;
        }
        else {
            while(page_pointer->appendPage != page && page_pointer) {
                // uart_puts("page: "); uart_hex(page_pointer); uart_newline();
                page_pointer = page_pointer->appendPage;
            }

            if(page_pointer == NULL) {
                raiseError("Cant relink usedPage\n");
            }

            page_pointer->appendPage = page->appendPage;
        }

        // uart_puts("Remove page: ");
        // uart_hex(page);
        // uart_newline();

        removeUsedAddress(page);
        freePage(page);
    }

    unlock_interrupt();
}

void removeUsedAddress(page_t* page) {
    uint32 idx = addressToBlockIdx(page->address);
    uint32 numCoverdPage = exp2(page->order);

    for(int i = 0; i < numCoverdPage; i++) {
        usedAddress[i + idx] = NULL; 
    }
}

void init_reserve() {
    uart_puts("Reserve memory\n");
    reserveMemory(0x0000, 0x1000);     // spin tables for multicore boot
    reserveMemory(LOAD_ADDR - _stack_size, heap_end);  // kernel and heap/stack space
    reserveMemory(INITRAMFS_ADDR, INITRAMFS_ADDR + MAX_INITRAMFS_SIZE);  // initramfs
}

void reserveMemory(uint64 start, uint64 end) {
    uint32 order = 0;
    uart_puts("Address: "); uart_hex(start); uart_puts(" to "); uart_hex(end); uart_newline();
    for(uint64 address = start; address <= end; address += MINI_PAGE_SIZE * KB) {
        // uart_puts("Reserve address: "); uart_hex(address); uart_newline();
        if(inRange(pagesAtOrder[MAX_ORDER - 1].firstFree, address)) {
            reservePage(order, address);
        }
        else {
            // uart_puts("Address: "); uart_hex(address); uart_puts(" is not covered by buddy system\n");
            break;
        }
    }
}

void reservePage(uint32 order, uint64 address) {
    page_t* waitFreePage = NULL;
    page_t* page_pointer;
    page_t* page;

    page = pagesAtOrder[order].firstFree;
    while(isNotPageHead(page)) {
        page = page->prevPage;
    }

    while(page != NULL) { 
        if(page->state == BUSY && inRange(page, address)) {
            // uart_puts("Already reserved, page: "); uart_hex(page); uart_newline();
            return;
        }
        page = page->nextPage;
    }

    do {
        page = getFreePageAtOrder(order);
        if(waitFreePage == NULL) {
            waitFreePage = page;
            page_pointer = page;
        }
        else {
            page_pointer->appendPage = page;
            page_pointer = page;
        }
    } while(!inRange(page, address));

    // uart_puts("Reserve page: "); uart_hex(page); uart_newline();
    if(waitFreePage != page) {
        page_pointer->appendPage = NULL;
        while(waitFreePage != NULL) {
            if(waitFreePage != page)
                freePage(waitFreePage);
            waitFreePage = waitFreePage->appendPage;
        }
    }
}

bool inRange(page_t* page, uint64 address) {
    return page->address <= address && address < (page->address + getPageSizeAtOrder(page->order));
}

bool inPool(memoryPool_t* pool, uint64 address) {
    return pool->address <= address && address < (pool->address + pool->size * BASE_NUM);
}