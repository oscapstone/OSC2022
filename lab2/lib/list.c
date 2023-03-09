#include "lib/list.h"

void INIT_LIST_HEAD(struct list_head* head){
    head->next = head;
    head->prev = head;
}

void list_add(struct list_head* node, struct list_head* head){
    struct list_head *next = head->next;

    next->prev = node;
    head->next = node;

    node->next = next;
    node->prev = head;
}

void list_add_tail(struct list_head* node, struct list_head* head){
    struct list_head* tail = head->prev;

    tail->next = node;
    head->prev = node;

    node->next = head;
    node->prev = tail;
}

void list_del(struct list_head* node){
    struct list_head* next = node->next;
    struct list_head* prev = node->prev;

    next->prev = prev;
    prev->next = next;
}
int list_empty(struct list_head* head){
    return (head->next == head);
}
