#include "list.h"
#include "uart.h"
#include "utils.h"
#define MEMORY_BASE 0x10000000
#define PAGESIZE 0x1000 // 4KB
#define MAX_PAGES 64
#define LOG2_MAX_PAGES 6
#define LOG2_MAX_PAGES_PLUS_1 7
#define BELONG_LEFT -1
#define ALLOCATED -2

struct buddy_system_node {
    int index;
    int level;
    struct list_head list;
} node[MAX_PAGES];

struct list_head head[LOG2_MAX_PAGES_PLUS_1];
int frame_array[MAX_PAGES];

void buddy_system_print_all() {
    struct list_head *tmp;
    uart_puts("----------- list ----------\n");
    for (int i = 0; i < LOG2_MAX_PAGES_PLUS_1; i++) {
        uart_puts("head[");
        uart_hex(i);
        uart_puts("]: ");
        list_for_each(tmp, &head[i]) {
            uart_hex(list_entry(tmp, struct buddy_system_node, list)->index);
            uart_puts(" ");
        }
        uart_puts("\n");
    }
    uart_puts("----------- array ----------");
    for (int i = 0; i < MAX_PAGES; i++) {
        if (i % 8 == 0)
            uart_puts("\n");
        if (frame_array[i] == BELONG_LEFT)
            uart_puts("--------");
        else if (frame_array[i] == ALLOCATED)
            uart_puts("XXXXXXXX");
        else
            uart_hex(frame_array[i]);
        uart_puts(" ");
    }
    uart_puts("\n----------------------------\n");

    return;
}

void buddy_system_init() {
    for (int i = 0; i < LOG2_MAX_PAGES_PLUS_1; i++)
        INIT_LIST_HEAD(&head[i]);
    for (int i = 0; i < MAX_PAGES; i++) {
        node[i].index = i;
        INIT_LIST_HEAD(&node[i].list);
        if (i == 0)
            frame_array[i] = LOG2_MAX_PAGES;
        else
            frame_array[i] = BELONG_LEFT;
    }
    node[0].level = LOG2_MAX_PAGES;
    list_add(&node[0].list, &head[LOG2_MAX_PAGES]);

    buddy_system_print_all();
    return;
}

void buddy_system_insert_list(struct list_head *node, struct list_head *head) {
    struct list_head *tmp;
    if (list_empty(head))
        list_add(node, head);
    else {
        int chk = 0;
        list_for_each(tmp, head) {
            if (list_entry(node, struct buddy_system_node, list)->index < list_entry(tmp, struct buddy_system_node, list)->index) {
                list_add_tail(node, tmp);
                chk = 1;
            }
        }
        if (!chk)
            list_add_tail(node, head);
    }
}

int buddy_system_find_suitable_size(int size) {
    int ret = 4;
    while (ret < size)
        ret <<= 1;
    return ret;
}

int buddy_system_spilt(int level) {
    if (level > LOG2_MAX_PAGES)
        return -1;

    if (list_empty(&head[level]) && buddy_system_spilt(level + 1) == -1)
        return -1;

    struct buddy_system_node *tmp = list_first_entry(&head[level], struct buddy_system_node, list);
    list_del_init(&tmp->list);
    frame_array[tmp->index] = level - 1;
    node[tmp->index].level = level - 1;
    frame_array[tmp->index + pow2(level - 1)] = level - 1;
    node[tmp->index + pow2(level - 1)].level = level - 1;
    list_add(&node[tmp->index + pow2(level - 1)].list, &head[level - 1]);
    list_add(&tmp->list, &head[level - 1]);
    return 1;
}

int buddy_system_find_node_index(int level) {
    if (list_empty(&head[level]) && buddy_system_spilt(level + 1) == -1)
        return -1;

    return list_first_entry(&head[level], struct buddy_system_node, list)->index;
}

void buddy_system_update_list(int index) {
    int level = frame_array[index];
    if (level == LOG2_MAX_PAGES)
        return;

    struct list_head *prev = node[index].list.prev;
    struct list_head *next = node[index].list.next;
    if (prev != &head[level]) {
        int prev_index = list_entry(prev, struct buddy_system_node, list)->index;
        if (prev_index / pow2(level + 1) == index / pow2(level + 1)) {
            list_del_init(&node[index].list);
            list_del_init(prev);
            frame_array[index] = BELONG_LEFT;
            frame_array[prev_index] = level + 1;
            node[prev_index].level = level + 1;
            list_add(prev, &head[level + 1]);
            buddy_system_update_list(prev_index);
        }
        return;
    }
    if (next != &head[level]) {
        int next_index = list_entry(next, struct buddy_system_node, list)->index;
        if (next_index / pow2(level + 1) == index / pow2(level + 1)) {
            list_del_init(&node[index].list);
            list_del_init(next);
            frame_array[next_index] = BELONG_LEFT;
            frame_array[index] = level + 1;
            node[index].level = level + 1;
            list_add(&node[index].list, &head[level + 1]);
            buddy_system_update_list(index);
        }
        return;
    }
    return;
}

unsigned long int buddy_system_alloc(int size) {
    int suitable_size = buddy_system_find_suitable_size(size);
    int level = log2(suitable_size / 4);
    uart_puts("buddy_system_alloc: ");
    uart_hex(suitable_size);
    uart_puts("\nlevel: ");
    uart_hex(level);
    uart_puts("\n");
    int index = buddy_system_find_node_index(level);
    if (index == -1) {
        uart_puts("Can't find suitable node!\n");
        return -1;
    }
    uart_puts("index: ");
    uart_hex(index);
    uart_puts("\n");
    list_del_init(&node[index].list);
    frame_array[index] = ALLOCATED;
    buddy_system_print_all();
    return MEMORY_BASE + index * PAGESIZE;
}

void buddy_system_free(int index) {
    uart_puts("buddy_system_free index: ");
    uart_hex(index);
    uart_puts("\n");

    if (frame_array[index] != ALLOCATED) {
        uart_puts("This node is not allocated!\n");
        return;
    }

    int level = node[index].level;
    frame_array[index] = level;
    uart_puts("level: ");
    uart_hex(level);
    uart_puts("\n");

    buddy_system_insert_list(&node[index].list, &head[level]);
    buddy_system_update_list(index);
    buddy_system_print_all();
}
