#ifndef MEMORY
#define MEMORY
#include "type.h"

extern unsigned long long _heap_start; // linker script symbol
extern unsigned long long _heap_end;  // linker script symbol
static char* heap = (char*)&_heap_start;
static char* heap_end = (char*)&_heap_end;

void* simple_malloc(unsigned int size);
char* memcpy (void *dest, const void *src, unsigned long long len);

#define MAX_ORDER 12
#define MAX_PAGES 2048
#define MINI_PAGE_SIZE 4 // kB
#define PAGE_BEGIN_ADDRESS 0x00000000
#define BASE_NUM 64 // can't change so far
#define NUM_POOL 1
#define KB 1 << 10

typedef struct node {
    struct node* prevNode;
    struct node* nextNode;
    uint64 address;
} node_t;

typedef struct memoryPool {
    uint64 address;
    uint32 size;
    uint64 stateBitMap;
    uint64 mask;
} memoryPool_t;

typedef struct page {
    uint32 order;
    uint64 address;
    int state;
    struct page* prevPage;
    struct page* nextPage;
    struct page* buddyPage;
    struct page* parentPage;
    struct page* appendPage;
    memoryPool_t* pool;
} page_t;

typedef struct pageHeader {
    page_t* firstFree;
    page_t* firstEmpty;
} pageHeader_t;

void initMemoryPages();
page_t* createPage(uint32 order, uint64 address, int state);
void initPool(memoryPool_t* pool, uint32 maxSize, uint32 size, uint64 address);
void* malloc(uint32 size);
uint32 sizeToSmallestOrder(uint32 size);
void appendNewPage(page_t* parentPage, page_t* newPage);
void showAppendPage(page_t *parentPage);
bool isAllFull(page_t* page, uint32 size);
bool isFull(page_t* page, uint32 size);
bool isPoolFull(memoryPool_t* pool, uint32 size);
void* allocateMemoryFromPage(page_t* page, uint32 size);
void registerUsedAddress(page_t* page);
uint32 addressToBlockIdx(uint64 address);
void removeUsedAddress(page_t* page);

page_t* getFreePageAtOrder(uint32 order);
void loggingPage(page_t* page);
void loggingPool(memoryPool_t* pool);
bool hasFreePageAtOrder(uint32 order);
bool hasNoFreePageAtOrder(uint32 order);
void dividePageToLowerOrder(page_t* page);
page_t* addPageAtOrder(uint32 order, uint64 address, page_t* parentPage);
uint64 getPageSizeAtOrder(uint32 order);
uint64 getPageNumAtOrder(uint32 order);
void bindAsBuddy(page_t* page1, page_t* page2);
void freePage(page_t* page);
bool isNotPageHead(page_t* page);
bool isNotPageTail(page_t* page);
bool isFirstFree(page_t* page);
bool isFirstEmpty(page_t* page);
void mergePagesToHigherOrder(page_t* page);
void moveToTail(page_t* page);
void loggingAllPageState();
void free(uint64 address);
void init_reserve();
void reserveMemory(uint64 start, uint64 end);
void reservePage(uint32 order, uint64 address);
bool inRange(page_t* page, uint64 address);
bool inPool(memoryPool_t* pool, uint64 address);

#endif