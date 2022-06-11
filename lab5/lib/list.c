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

int list_is_last(struct list_head* node, struct list_head* head){
    return head->prev == node;
}

int list_is_head(struct list_head* node, struct list_head* head){
    return head == node;
}

void list_splice(struct list_head *list, struct list_head *head){
    struct list_head *head_first = head->next;
    struct list_head *list_first = list->next;
    struct list_head *list_last = list->prev;

    if (list_empty(list))
        return;

    head->next = list_first;
    list_first->prev = head;

    list_last->next = head_first;
    head_first->prev = list_last;
}

void list_splice_tail(struct list_head *list, struct list_head *head){
    struct list_head *head_last = head->prev;
    struct list_head *list_first = list->next;
    struct list_head *list_last = list->prev;

    if (list_empty(list))
        return;

    head->prev = list_last;
    list_last->next = head;

    list_first->prev = head_last;
    head_last->next = list_first;
}
