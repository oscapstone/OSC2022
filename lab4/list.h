#ifndef LIST_H
#define LIST_H
typedef struct _list {
	struct _list* next;
	struct _list* prev;
}_list;

void pop_list(_list* chunk);
void init_list(_list* l);
void push_list(_list* l, _list* chunk);
int empty_list(_list* l);

#endif