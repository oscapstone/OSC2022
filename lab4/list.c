#include "list.h"

void pop_list(_list* chunk) {
	chunk->next->prev = chunk->prev;
	chunk->prev->next = chunk->next;
}

void init_list(_list* l) {
	l->next = l;
	l->prev = l;
}

void push_list(_list* l, _list* chunk){
	chunk->next = l->next;
	chunk->prev = l;
	l->next->prev = chunk;
	l->next = chunk;
}

int empty_list(_list* l) {
	return l->next == l;
}