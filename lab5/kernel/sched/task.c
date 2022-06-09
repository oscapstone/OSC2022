#include "kernel/sched/task.h"

extern void run_init_task_load_all(void);
void task_init(){
}
uint64_t task_create(){}
void task_exit(){}
void task_destroy(struct task_struct*){}
void run_init_task(char* filename){
    uint64_t size;
    struct vm_area_struct* pvma;
    uint64_t user_entry, user_sp;
    struct trap_frame* ptrap_frame;
    LOG("run_init_task enter");
    struct task_struct* task = (struct task_struct*)kmalloc(sizeof(struct task_struct));

    // initialize mm
    // load executable 
    size = initrdfs_filesize(filename);
    task->mm = kmalloc(sizeof(struct mm_struct));
    INIT_LIST_HEAD(&task->mm->mmap_list);
    // create text memory area
    pvma = kmalloc(sizeof(struct vm_area_struct));
    pvma->vm_start = alloc_pages(BUDDY_MAX_ORDER - 1);
    pvma->vm_end = pvma->vm_start + ALIGN_UP(size, PAGE_SIZE);
    list_add_tail(&pvma->list, &task->mm->mmap_list);
    // copy file to text memory area
    initrdfs_loadfile(filename, pvma->vm_start);
    user_entry = (uint64_t)pvma->vm_start;
    // create user stack
    pvma = kmalloc(sizeof(struct vm_area_struct));
    pvma->vm_start = alloc_pages(1);
    pvma->vm_end = pvma->vm_start + PAGE_SIZE * 2;
    list_add_tail(&pvma->list, &task->mm->mmap_list);
    user_sp = (uint64_t)pvma->vm_end;

    // create kernel stack
    task->stack = alloc_pages(1);
    
    // initialize trap frame in kernel stack
    ptrap_frame = (struct trap_frame*)((uint64_t)task->stack + PAGE_SIZE * 2 - sizeof(struct trap_frame));
    ptrap_frame->spsr_el1 = 0x0; 
    ptrap_frame->sp_el0 = user_sp; 
    ptrap_frame->x29 = user_sp;
    ptrap_frame->elr_el1 = user_entry;
    ptrap_frame->x30 = user_entry;

    // initialize thread_info
    task->thread_info.pid = get_pid_counter();
    task->thread_info.state = TASK_RUNNING;

    // initialize thread context
    task->ctx.lr = (uint64_t)run_init_task_load_all;
    task->ctx.sp = (uint64_t)task->stack + PAGE_SIZE * 2 - sizeof(struct trap_frame);
    task->ctx.fp = task->ctx.sp;


    // initialize relationship
    task->parent = NULL;
    task->child = NULL;
    INIT_LIST_HEAD(&task->siblings);

    // initialize schedule info
    task->sched_info.rticks = 0;
    task->sched_info.priority = 1;
    task->sched_info.counter = task->sched_info.priority;
    
    LOG("run_init_task end");
    add_task_to_rq(task);
}


