#include "kernel/sched/task.h"

extern void task_load_all(void);

void task_init(){
}

uint64_t task_dup(struct task_struct* parent){
    LOG("task_dup start");
    volatile uint64_t daif;
    struct task_struct* child = (struct task_struct*)kmalloc(sizeof(struct task_struct));
    struct vm_area_struct* pvma, *ppvma;
    struct vm_area_struct_list* pvmal, *ppvmal;
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
        ppvmal = list_entry(node, struct vm_area_struct_list, list);
        ppvma = ppvmal->vm_area;
        
        LOG("parent's vm area type 0x%x", ppvma->type);
        if(ppvma->type == VM_AREA_STACK){
            // user stack
            pvma = kmalloc(sizeof(struct vm_area_struct));
            pvmal = kmalloc(sizeof(struct vm_area_struct_list));
            pvma->type = ppvma->type;
            pvma->vm_start = (uint64_t)alloc_pages(1);
            pvma->vm_end = pvma->vm_start + PAGE_SIZE * 2;
            pvma->ref = 1;
            pvmal->vm_area = pvma;

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
            pvmal = kmalloc(sizeof(struct vm_area_struct_list));
            pvmal->vm_area = ppvma;
            ppvma->ref++;
        }
        list_add_tail(&pvmal->list, &child->mm->mmap_list);
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

    // initialzie singal
    sigpending_init(&child->sigpending);
    
    // insert task to run queue and task list
    list_add_tail(&child->list, &task_list);
    local_irq_restore(daif);
    add_task_to_rq(child);
    LOG("task_dup end");
    return child_pid;
}

void task_kill(uint64_t pid){
    LOG("task_kill start");
    volatile uint64_t daif;
    struct task_struct* target;
    struct task_struct* current;

    target = find_task_by_pid(pid);
    if(target == NULL || (target->thread_info.state == TASK_DEAD)) return; 

    current = get_current();

    daif = local_irq_disable_save();

    target->thread_info.state = TASK_DEAD;
    list_add_tail(&target->zombie, &zombies);
    if(target != current){
        list_del(&target->sched_info.sched_list);
    }

    local_irq_restore(daif);

    LOG("task_kill end");

    preempt_schedule();
}

void task_exit(){
    volatile uint64_t daif;
    LOG("task_exit start");
    struct task_struct* cur;
    cur = get_current();
    cur->thread_info.state = TASK_DEAD;

    daif = local_irq_disable_save();
    list_add_tail(&cur->zombie, &zombies);
    local_irq_restore(daif);
    LOG("task_exit end");
    preempt_schedule();
}

void task_destroy(struct task_struct* task){
    struct vm_area_struct *vm_area;
    struct vm_area_struct_list *vm_area_list;
    struct list_head* node;
    struct task_struct* child;
    
    list_del(&task->list);

    // free kernel stack
    free_pages(task->stack, 1);

    // free mm
    while(!list_empty(&task->mm->mmap_list)){
        vm_area_list = list_first_entry(&task->mm->mmap_list, struct vm_area_struct_list, list);
        vm_area = vm_area_list->vm_area; 
        vm_area->ref--;

        if(vm_area->ref <= 0){
            if(vm_area->type == VM_AREA_PROGRAM) free_pages((void*)vm_area->vm_start, BUDDY_MAX_ORDER - 1);
            else if(vm_area->type == VM_AREA_STACK) free_pages((void*)vm_area->vm_start, 1);
            kfree(vm_area);
        }

        list_del(&vm_area_list->list); 
        kfree(vm_area_list);
    }
    kfree(task->mm); 
     
    //unlink from parent and sibliings and set re-parent
    if(task->parent){
        //unlink from parent
        if(task->parent->child == task){
            child = list_first_entry(&task->siblings, struct task_struct, siblings);
            task->parent->child = child;
            list_del(&task->siblings);
        }
        // re-parent
        if(task->child){
            task->child->parent = task->parent;
            list_for_each(node, &task->child->siblings){
                child = list_entry(node, struct task_struct, siblings);
                child->parent = task->parent;
            }
            list_splice_tail(&task->child->siblings, &task->parent->child->siblings);
            list_add_tail(&task->child->siblings, &task->parent->child->siblings);
        }
    }

    kfree(task);
    if(user_init == task) user_init = NULL;
}

int task_exec(const char* name, char* const argv[]){
    LOG("enter task_exec");
    LOG("file name: %s", name);
    int ret = -1; 
    struct list_head* node;
    struct vm_area_struct *pvma;
    struct vm_area_struct_list *pvmal;
    struct task_struct* current = get_current();
    struct trap_frame *trap_frame = get_trap_frame(current);
    volatile uint64_t daif;
    size_t s;

    daif = local_irq_disable_save();
    // initialize mm
    if(s = initrdfs_filesize((char*)name)){
        list_for_each(node, &current->mm->mmap_list){
            LOG("node[-1]: %x", ((uint64_t*)node)[-1]);
            pvmal = list_entry(node, struct vm_area_struct_list, list);
            LOG("pvmal: %x", pvmal);
            pvma = pvmal->vm_area;
            LOG("pvma: %x", pvma);
            if(pvma->type == VM_AREA_STACK){
                trap_frame->sp_el0 = pvma->vm_end;
            }else if(pvma->type == VM_AREA_PROGRAM){
                pvma->ref--;
                if(pvma->ref <= 0){
                    free_pages((void*)pvma->vm_start, BUDDY_MAX_ORDER - 1);
                }else{
                    pvma = kmalloc(sizeof(struct vm_area_struct));
                }
                pvmal->vm_area = pvma;
                pvma->vm_start = (uint64_t)alloc_pages(BUDDY_MAX_ORDER - 1);
                trap_frame->elr_el1 = pvma->vm_start;
                initrdfs_loadfile((char*)name, (void*)pvma->vm_start);
                pvma->vm_end = pvma->vm_start + (1 << (BUDDY_MAX_ORDER - 1)) * PAGE_SIZE; 
                pvma->type = VM_AREA_PROGRAM;
                pvma->ref = 1;
            }else{
                printf("unkown vm area\r\n");
                while(1);
            }
        }
        trap_frame->spsr_el1 = 0;  
        ret = 0;
    }
    // initialize signal
    default_sighand_init(&current->sighandler);

    local_irq_restore(daif);
    LOG("end task_exec");
    return ret;
}

void run_init_task(char* filename){
    uint64_t size;
    struct vm_area_struct* pvma;
    struct vm_area_struct_list* pvmal;
    uint64_t user_entry, user_sp;
    struct trap_frame* ptrap_frame;
    volatile uint64_t daif;
    LOG("run_init_task enter");
    LOG("Size of struct trap_frame: %x", sizeof(struct trap_frame));

    daif = local_irq_disable_save();
    struct task_struct* task = (struct task_struct*)kmalloc(sizeof(struct task_struct));

    // initialize mm
    // load executable 
    size = initrdfs_filesize(filename);
    task->mm = kmalloc(sizeof(struct mm_struct));
    INIT_LIST_HEAD(&task->mm->mmap_list);
    // create text memory area
    pvma = kmalloc(sizeof(struct vm_area_struct));
    pvmal = kmalloc(sizeof(struct vm_area_struct_list));
    pvma->vm_start = (uint64_t)alloc_pages(BUDDY_MAX_ORDER - 1);
    pvma->vm_end = pvma->vm_start + (1 << (BUDDY_MAX_ORDER - 1)) * PAGE_SIZE;
    pvma->ref = 1;
    pvma->type = VM_AREA_PROGRAM;
    pvmal->vm_area = pvma;
    list_add_tail(&pvmal->list, &task->mm->mmap_list);
    // copy file to text memory area
    initrdfs_loadfile(filename, (void*)pvma->vm_start);
    user_entry = (uint64_t)pvma->vm_start;
    LOG("pvmal: %x", pvmal);
    // create user stack
    pvma = kmalloc(sizeof(struct vm_area_struct));
    pvmal = kmalloc(sizeof(struct vm_area_struct_list));
    pvma->vm_start = (uint64_t)alloc_pages(1);
    pvma->vm_end = pvma->vm_start + PAGE_SIZE * 2;
    pvma->ref = 1;
    pvma->type = VM_AREA_STACK;
    pvmal->vm_area = pvma;
    list_add_tail(&pvmal->list, &task->mm->mmap_list);
    user_sp = (uint64_t)pvma->vm_end;

    LOG("pvmal: %x", pvmal);
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

    // initialzie singal
    sigpending_init(&task->sigpending);
    default_sighand_init(&task->sighandler);
    
    // insert task to run queue and task list
    user_init = task;
    list_add_tail(&task->list, &task_list);
    local_irq_restore(daif);

    LOG("run_init_task end");
    add_task_to_rq(task);
}

uint64_t sys_fork(){
    return task_dup(get_current());
}

uint64_t sys_getpid(){
    return get_current()->thread_info.pid;
}

void sys_exit(uint64_t status){
    task_exit(status);
}

void sys_kill(uint64_t pid){
    task_kill(pid);
}
uint64_t sys_exec(const char* name, char *const argv[]){
    return task_exec(name, argv);
}
