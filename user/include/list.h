#ifndef __TOOLS_LINUX_LIST_H
#define __TOOLS_LINUX_LIST_H
struct list_head {
    struct list_head *next, *prev;
};
#define WRITE_ONCE(var, val) \
    (*((volatile typeof(val) *)(&(var))) = (val))
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	WRITE_ONCE(list->next, list);
	list->prev = list;
}
static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next) 
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}
static inline void list_add(struct list_head *new, struct list_head *head){
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head){
    __list_add(new, head->prev, head);
}

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)
    //  in include/linux/kernel.h

/**
* container_of – cast a member of a structure out to the containing structure
* @ptr:    the pointer to the member.
* @type:    the type of the container struct this is embedded in.
* @member:    the name of the member within the struct.
*
*/
#define container_of(ptr, type, member) ({            \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr – offsetof(type,member) );})

#endif