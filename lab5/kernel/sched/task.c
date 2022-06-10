#include "kernel/sched/task.h"

extern void task_load_all(void);

void task_init(){
}

uint64_t task_dup(struct task_struct* parent){
    LOG("task_dup start");
    uint64_t daif;
    struct task_struct* child = (struct task_struct*)kmalloc(sizeof(struct task_struct));
    struct vm_area_struct* pvma, *ppvma;
    struct list_head *node;
    struct trap_frame *pptrap_frame;
    struct trap_frame *pctrap_frame;
    uint64_t child_pid;
    uint64_t user_sp, kernel_sp;
    uint64_t user_fp, kernel_fp;


    daif = local_irq_disable_save();
    
    // fix child's kernel stack
    child->stack = alloc_pages(1);
    memcpy(child->stack, parent->stack, PAGE_SIZE * 2); 

    pptrap_frame = get_trap_frame(parent);
    // initialize mm
    child->mm = kmalloc(sizeof(struct mm_struct));
    INIT_LIST_HEAD(&child->mm->mmap_list);
    list_for_each(node, &parent->mm->mmap_list){
        pvma = kmalloc(sizeof(struct vm_area_struct));
        ppvma = list_entry(node, struct vm_area_struct, list);
        
        pvma->type = ppvma->type;
        LOG("parent's vm area type 0x%x", ppvma->type);
        if(ppvma->type == VM_AREA_STACK){
            // user stack
            pvma->vm_start = (uint64_t)alloc_pages(1);
            pvma->vm_end = pvma->vm_start + PAGE_SIZE * 2;

            LOG("parent user stack 0x%x", ppvma->vm_end);
            LOG("Create child user stack 0x%x", pvma->vm_end);

            memcpy((void*)pvma->vm_start, (void*)ppvma->vm_start, PAGE_SIZE * 2);
            user_sp = pvma->vm_end - (ppvma->vm_end - pptrap_frame->sp_el0);
            LOG("parent's sp 0x%x", pptrap_frame->sp_el0);
            LOG("user_sp 0x%x", user_sp);

            user_fp = pvma->vm_end - (ppvma->vm_end - pptrap_frame->x29);
            LOG("parent's fp 0x%x", pptrap_frame->x29);
            LOG("user_fp 0x%x", user_fp);


        }else{
            pvma->vm_start = ppvma->vm_start;
            pvma->vm_end = ppvma->vm_end;
        }
        list_add_tail(&pvma->list, &child->mm->mmap_list);
    }

    // fix child's trap frame
    pctrap_frame = get_trap_frame(child);
    LOG("parent kernel stack: %x\r\n", parent->stack + PAGE_SIZE * 2);
    LOG("child trap stack: %x\r\n", child->stack + PAGE_SIZE * 2);

    LOG("parent trap frame: %x\r\n", pptrap_frame);
    LOG("child trap frame: %x\r\n", pctrap_frame);
    pctrap_frame->x29 = user_fp;
    pctrap_frame->sp_el0 = user_sp;
    pctrap_frame->spsr_el1 = 0;
    // set child's return value to 0
    pctrap_frame->x0 = 0;

    // initialize thread_info
    child->thread_info.pid = get_pid_counter();
    child_pid = child->thread_info.pid;
    child->thread_info.state = TASK_RUNNING;

    // initialize thread context
    // directly back to user space
    memcpy(&child->ctx,  &parent->ctx, sizeof(struct task_ctx));
    child->ctx.lr = (uint64_t)task_load_all;
    child->ctx.sp = (uint64_t)pctrap_frame;
    child->ctx.fp = (uint64_t)pctrap_frame; 

    // initialize child's relationship
    child->parent = parent;
    child->child = NULL;
    INIT_LIST_HEAD(&child->siblings);

    // initialize parent's relationship
    if(parent->child != NULL){
        list_add_tail(&child->siblings, &parent->child->siblings);
    }else{
        parent->child = child;
    }

    // initialize schedule info
    child->sched_info.rticks = 0;
    child->sched_info.priority = parent->sched_info.priority ;
    child->sched_info.counter = child->sched_info.priority;
   
    add_task_to_rq(child);
    local_irq_restore(daif);
    LOG("task_dup end");
    return child_pid;
}


void task_exit(){}
void task_destroy(struct task_struct*){}
void run_init_task(char* filename){
    uint64_t size;
    struct vm_area_struct* pvma;
    uint64_t user_entry, user_sp;
    struct trap_frame* ptrap_frame;
    LOG("run_init_task enter");
    LOG("Size of struct trap_frame: %x", sizeof(struct trap_frame));
    struct task_struct* task = (struct task_struct*)kmalloc(sizeof(struct task_struct));

    // initialize mm
    // load executable 
    size = initrdfs_filesize(filename);
    task->mm = kmalloc(sizeof(struct mm_struct));
    INIT_LIST_HEAD(&task->mm->mmap_list);
    // create text memory area
    pvma = kmalloc(sizeof(struct vm_area_struct));
    pvma->vm_start = (uint64_t)alloc_pages(BUDDY_MAX_ORDER - 1);
    pvma->vm_end = pvma->vm_start + ALIGN_UP(size, PAGE_SIZE);
    pvma->type = 0;
    list_add_tail(&pvma->list, &task->mm->mmap_list);
    // copy file to text memory area
    initrdfs_loadfile(filename, (void*)pvma->vm_start);
    user_entry = (uint64_t)pvma->vm_start;
    // create user stack
    pvma = kmalloc(sizeof(struct vm_area_struct));
    pvma->vm_start = (uint64_t)alloc_pages(1);
    pvma->vm_end = pvma->vm_start + PAGE_SIZE * 2;
    pvma->type = VM_AREA_STACK;
    list_add_tail(&pvma->list, &task->mm->mmap_list);
    user_sp = (uint64_t)pvma->vm_end;

    // create kernel stack
    task->stack = alloc_pages(1);
    
    // initialize trap frame in kernel stack
    ptrap_frame = (struct trap_frame*)get_trap_frame(task);
    ptrap_frame->spsr_el1 = 0x0; 
    ptrap_frame->sp_el0 = user_sp; 
    ptrap_frame->elr_el1 = user_entry;

    // initialize thread_info
    task->thread_info.pid = get_pid_counter();
    task->thread_info.state = TASK_RUNNING;

    // initialize thread context
    task->ctx.lr = (uint64_t)task_load_all;
    task->ctx.sp = (uint64_t)get_trap_frame(task);
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

uint64_t sys_fork(){
    return task_dup(get_current());
}

uint64_t sys_getpid(){
    return get_current()->thread_info.pid;
}
