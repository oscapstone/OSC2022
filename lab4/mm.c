#include "mm.h"
#include "list.h"
#include "uart.h"
extern unsigned char __start, __end;
unsigned long reserve_count = 0;

void init_memory_system() {
    buddy_state = (char*)&__end;
    init_startup();
    init_buddy_system();
}
void init_buddy_system() {
 
	int num_of_pages = MEMORY_SIZE / PAGE_SIZE;
    int i;
	for (i = 0; i < num_of_pages; i++)
		buddy_state[i] = INIT_PAGE;
	for (i = 0; i < MAX_BUDDY_ORDER; i++)
		init_list(&(buddy.free_list[i]));
    unsigned long mem_itr = 0;
    for (int i = 0; i < reserve_count; i++) {
        place_buddy(mem_itr, startup.addr[i], BUDDY_FREE);
        place_buddy(startup.addr[i], startup.addr[i] + startup.size[i], RESERVE_USE);
        mem_itr = startup.addr[i] + startup.size[i];
    }
	place_buddy(mem_itr, MEMORY_SIZE, BUDDY_FREE);
}

int reserve_mem(unsigned long addr, unsigned long size) {
  //uart_put_int(reserve_count);
  //uart_puts("\n");
  if ((addr & 0xfff) != 0 || (size & 0xfff) != 0) {
    uart_puts("reserve mem require page align\n");
    return -1;
  }
  if (reserve_count >= STARTUP_MAX) {
    uart_puts("no reserve slot available\n");
    return -1;
  } else {
    for (int i = 0; i < reserve_count; i++) {
      if (check_reserve_collision((unsigned long)startup.addr[i], startup.size[i], (unsigned long)addr, size)) {
        uart_puts("reserve collision\n");
        return -1;
      }
    }
    startup.addr[reserve_count] = addr;
    startup.size[reserve_count] = size;
    reserve_count++;
    return 0;
  }
}

unsigned long check_reserve_collision(unsigned long a1, unsigned int s1, unsigned long a2, unsigned int s2) {
  unsigned long e1 = (unsigned long)(a1 + s1);
  unsigned long e2 = (unsigned long)(a2 + s2);
  return ((a2 >= a1) && (a2 < e1)) || ((e2 > a1) && (e2 <= e1)) ||
         ((a1 >= a2) && (a1 < e2)) || ((e1 > a2) && (e1 <= e2));
}

void init_startup(){
	reserve_count = 0;
	//uart_puts("reserve spin table\n");
	reserve_mem(0x0, 0x1000);  														// spin table
	//uart_puts("spin table reserve finish\n");
	//uart_puts("reserve kernel\n");
	reserve_mem((unsigned long)&__start, (unsigned long)(&__end - &__start)); 		// kernel
	//uart_puts("kernel reserve finish\n");
	//uart_puts("reserve buddy system\n");
	reserve_mem((unsigned long)&__end, MEMORY_SIZE / PAGE_SIZE);     					// buddy system
	//uart_puts("buddy system reserve finish\n");
	//uart_puts("reserve cpio\n");
	reserve_mem(0x30000000,0x100000);												// cpio
	//uart_puts("cpio reserve finish\n");
	//uart_puts("reserve dtb\n");
	reserve_mem(0x31000000,0x1000000);                  							// dtb
	//uart_puts("dtb reserve finish\n");
}

void place_buddy(unsigned long st, unsigned long ed, int flag) {
    while (st != ed) {
        int st_ord = get_order(st);
        int ed_ord = get_order(ed);
        if (st_ord >= MAX_BUDDY_ORDER) {
            st_ord = MAX_BUDDY_ORDER - 1;
        }
        if (ed_ord >= MAX_BUDDY_ORDER) {
            ed_ord = MAX_BUDDY_ORDER - 1;
        }
        if (st_ord <= ed_ord) {
            _place_buddy(st, st_ord, flag);
            st += (1 << (st_ord + PAGE_SIZE_CTZ));
        }
        else {
            ed -= (1 << (ed_ord + PAGE_SIZE_CTZ));
            _place_buddy(ed, ed_ord, flag);
        }
    }
}

void _place_buddy(unsigned long ptr, int ord, int flag) {
    unsigned long idx = ptr >> PAGE_SIZE_CTZ;
    set_buddy_ord(buddy_state[idx], ord);
    set_buddy_flag(buddy_state[idx], flag);
    if (flag == BUDDY_FREE) {
        push_list(&buddy.free_list[ord], (_list*)ptr);
    }
}

void print_buddy_info() {
    uart_puts("***********************************\n");
    uart_puts("buddy_system\n");
    uart_puts("order \tnum\n");
    for (int i = 0; i < MAX_BUDDY_ORDER; i++) {
        uart_put_int(i);
        uart_puts(" \t");
        _list* header = &(buddy.free_list[i]);
        _list* l = header;
        int count = 0;
        while (l->next != header) {
            count++;
            l = l->next;
        }
        uart_put_int(count);
        uart_puts("\n");
    }
    uart_puts("***********************************\n");
}

void print_buddy_stat() {
    uart_puts("***********************************\n");
    uart_puts("page_address\tstate\n");
    for (int i = 0; i < MEMORY_SIZE / PAGE_SIZE; i++) {
        if(get_buddy_ord(buddy_state[i])<MAX_BUDDY_ORDER){
            int state = get_buddy_flag(buddy_state[i]);
            uart_puts("0x");
            uart_put_hex(i);
            if (state == BUDDY_USE) {
                uart_puts("\tBUDDY_USE\n");
            }else if (state == BUDDY_FREE) {
                uart_puts("\tBUDDY_FREE\n");
            }else if (state == RESERVE_USE) {
                uart_puts("\tRESERVE_USE\n");
            }
        }
    }
    uart_puts("***********************************\n");
}

void *page_allocate(unsigned int size){
    uart_puts("allocate_size: 0x");
    uart_put_hex((unsigned long)size);
    uart_puts("\n");
    size = pad(size, PAGE_SIZE);
    int target_order = 51 - __builtin_clzl(size);
    if ((((1 << (target_order + 12)) - 1) & size) != 0)
        target_order++;
    int find_order = target_order;
    while(empty_list(&buddy.free_list[find_order])){
        find_order++;
        if (find_order >= MAX_BUDDY_ORDER) {
            uart_puts("out of memory\n");
            return NULL;
        }
    }
    _list *chunk = buddy.free_list[find_order].next;
    pop_list(chunk);
    int page_num = ptr_to_pagenum(chunk);
    set_buddy_flag(buddy_state[page_num], BUDDY_USE);
    set_buddy_ord(buddy_state[page_num], target_order);
    while(find_order>target_order){
        find_order--;
        unsigned int bd = buddy_pagenum(page_num, find_order);
        set_buddy_flag(buddy_state[bd], BUDDY_FREE);
        set_buddy_ord(buddy_state[bd], find_order);
        push_list(&buddy.free_list[find_order], (_list *)pagenum_to_ptr(bd));
        uart_puts("releasing redundant memory, address: 0x");
        uart_put_hex((unsigned long)pagenum_to_ptr(bd));
        uart_puts(", size: 0x");
        uart_put_hex((unsigned long)(1<<find_order)*4096);
        uart_puts("byte\n");
    }
    uart_puts("allocate_address: ");
    uart_put_hex((unsigned long)chunk);
    uart_puts("\n");
    return chunk;
}

void page_free(void *ptr){
  uart_puts("free page: ");
  uart_put_hex((unsigned long)ptr);
  uart_puts("\n");
  unsigned long pagenum = ptr_to_pagenum(ptr);
  unsigned long ord = get_buddy_ord(buddy_state[pagenum]);
  buddy_state[pagenum] = INIT_PAGE;
  //merge
  while (ord < MAX_BUDDY_ORDER - 1) {
    unsigned long buddy = buddy_pagenum(pagenum, ord);
    if (get_buddy_flag(buddy_state[buddy]) == BUDDY_FREE &&
        get_buddy_ord(buddy_state[buddy]) == ord) {
      uart_puts("coalesce\n");
      pop_list((_list *)pagenum_to_ptr(buddy));
      buddy_state[buddy] = INIT_PAGE;
      ord++;
      pagenum = pagenum < buddy ? pagenum : buddy;
    } else {
      break;
    }
  }
  set_buddy_flag(buddy_state[pagenum], BUDDY_FREE);
  set_buddy_ord(buddy_state[pagenum], ord);
  push_list(&buddy.free_list[ord], pagenum_to_ptr(pagenum));
}

void test_buddy(){
	print_buddy_info();
	char*p = page_allocate(128*4096);
	print_buddy_info();
	page_free(p);
	print_buddy_info();
}

