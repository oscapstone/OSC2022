#include "task.h"
#include "exception.h"
#include "timer.h"
void init_taskq(){
    taskq_head = nullptr;
    return;
}
void add_task(void (*handler)(),int privilege){
    taskq *node = (taskq*)simple_malloc(sizeof(taskq));
    node->handler = handler;
    node->privilege = privilege;
    node->next = nullptr;
    if(taskq_head==nullptr){
        taskq_head = node;
    }
    else{
        // taskq* newNode = (taskq*)simple_malloc(sizeof(taskq));

        if(privilege < taskq_head->privilege){
            node->next = taskq_head;
            taskq_head = node;
            // set_expired_time(after);
        }
        else{
            taskq *itr_node = taskq_head;
            while(itr_node->next!=null){
                if(itr_node->next->privilege>=privilege){
                    break;
                }
                itr_node = itr_node->next;
            }
            node->next = itr_node->next;
            itr_node->next = node;
        }
    }
    return;
}
void do_task(int* curr_task_privilege){
    while(!is_taskq_empty()){
        disable_interrupt();
        
        taskq *node = taskq_head;
        int last_task_privilege = *curr_task_privilege;
        *curr_task_privilege = node->privilege;
        taskq_head = taskq_head->next;
            
        enable_interrupt();

        node->handler();
        disable_interrupt();
        *curr_task_privilege = last_task_privilege;
        enable_interrupt();
    }
}

bool is_taskq_empty(){
    if(taskq_head == nullptr)
        return TRUE;
    else
        return FALSE;
}