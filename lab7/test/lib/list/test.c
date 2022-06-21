#include "lib/list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
struct student{
    int id;
    char name[100];
    struct list_head list;
};

struct student* create_student(int id, char* name){
    struct student* s = (struct student*)malloc(sizeof(struct student));
    s->id = id;
    strcpy(s->name, name);
    return s;
}
void print_student(struct student* s){
    printf("------------------------------------------\n");
    printf(" id: %d\n", s->id);
    printf(" name: %s\n", s->name);
    printf("------------------------------------------\n");
}
int main(void){
    struct list_head* node, head;
    INIT_LIST_HEAD(&head);

    if(list_empty(&head)){
        printf("List is empty\n");
    }
    list_add(&create_student(13, "xiaobye")->list, &head);
    list_add(&create_student(35, "test01")->list, &head);
    list_add(&create_student(34, "test02")->list, &head);

    list_for_each(node, &head){
        struct student* s = list_entry(node, struct student, list);
        print_student(s);
    }
    if(!list_empty(&head)){
        list_del(head.next);
        list_for_each(node, &head){
            struct student* s = list_entry(node, struct student, list);
            print_student(s);
        }
        list_del(head.next);
        list_for_each(node, &head){
            struct student* s = list_entry(node, struct student, list);
            print_student(s);
        }
        list_del(head.next);
        list_for_each(node, &head){
            struct student* s = list_entry(node, struct student, list);
            print_student(s);
        }
    }


    return 0;
}

