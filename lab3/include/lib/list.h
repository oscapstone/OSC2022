#ifndef _LIST_H_
#define _LIST_H_

#include "types.h"
struct list_head{
   struct list_head* next, *prev; 
}__attribute__((packed));

#define LIST_HEAD(head) struct list_header head = { &(head), &(head) }
#define list_entry(node, type, member) container_of(node, type, member)
#define list_first_entry(head, type, member) list_entry((head)->next, type, member)
#define list_last_entry(head, type, member) list_entry((head)->prev, type, member)
#define list_for_each(node, head) \
    for(node = (head)->next ; node != (head) ; node = node->next)

extern void INIT_LIST_HEAD(struct list_head*);
extern void list_add(struct list_head* node, struct list_head*);
extern void list_add_tail(struct list_head*, struct list_head*);
extern void list_del(struct list_head*);
extern int list_empty(struct list_head*);
#endif
