#ifndef __LIST_H__
#define __LIST_H__

#include "stddef.h"

struct list_head {
    struct list_head* prev;
    struct list_head* next;
};

void INIT_LIST_HEAD(struct list_head *head);
int list_empty(const struct list_head *head);
void list_add_tail(struct list_head *node, struct list_head *head);
void list_del(struct list_head *node);
struct list_head* list_pop(struct list_head *head);

#endif