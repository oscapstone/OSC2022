
#include "list.h"

void INIT_LIST_HEAD(struct list_head *head)
{
    head->next = head; head->prev = head;
}

int list_empty(const struct list_head *head) 
{
    return (head->next == head);
}

void list_add_tail(struct list_head *node, struct list_head *head)
{
    struct list_head *prev = head->prev;

    prev->next = node;
    node->next = head;
    node->prev = prev;
    head->prev = node;
}

void list_del(struct list_head *node)
{
    // uart_shex("node = 0x", node,"\n");

    struct list_head *next = node->next;
    struct list_head *prev = node->prev;

    // uart_shex("next = 0x", next, "\n");
    // uart_shex("prev = 0x", prev, "\n\n");

    // uart_shex("node->next = 0x", node->next, "\n");
    // uart_shex("node->prev = 0x", node->prev, "\n\n");

    // uart_shex("*node = 0x", *((uint32_t *)(node)), "\n");

    // uint32_t *addr = 0x06000000;
    // uart_shex("addr = 0x", addr, "\n");
    // uart_shex("*addr = 0x", *addr, "\n");

    // next = 0x06000000;
    // uart_sdec("size = 0x", sizeof(next), "\n\n");
    // uart_sdec("*size = 0x", sizeof(*next), "\n\n");
    // uart_sdec("*size = 0x", sizeof(void*), "\n\n");
    // uart_shex("*next = 0x", *((uint32_t *)(next)), "\n\n");
    if (next != NULL)
        next->prev = prev;
    if (prev != NULL)
        prev->next = next;
}

struct list_head* list_pop(struct list_head *head)
{
    struct list_head* node;
    node = head->next;
    list_del(node);

    return node;
}