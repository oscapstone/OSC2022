#ifndef __LIST__H__
#define __LIST__H__

typedef struct list_head {
	struct list_head *next, *prev;
} list_head_t;

static inline void list_head_init(list_head_t *list){
	list->prev = list;
	list->next = list;
}

static inline int list_empty(list_head_t *head){
	return head->next == head;
}

static inline void list_remove(list_head_t *list_a, list_head_t *list_b){
	list_a->prev->next = list_b->next;
    list_b->next->prev = list_a->prev;
}

static inline void list_insert_new(list_head_t *new_node, list_head_t *prev_node, list_head_t *next_node){

	next_node->prev = new_node;
	new_node->next = next_node;
	new_node->prev = prev_node;
	prev_node->next = new_node;

}

static inline void list_insert_next(list_head_t *new_node, list_head_t *head){
	list_insert_new(new_node, head, head->next);
}

static inline void list_insert_prev(list_head_t *new_node, list_head_t *head){
	list_insert_new(new_node, head->prev, head);
}

#endif