#include "kernel/sched/task.h"

extern void task_load_all(void);
extern void switch_ttbr0(uint64_t);

void task_init(){
}

uint64_t task_dup(struct task_struct* parent){
    LOG("task_dup start");
    volatile uint64_t daif;
    struct task_struct* child = (struct task_struct*)kmalloc(sizeof(struct task_struct));
    struct vm_area_struct* pvma, *ppvma;
    struct trap_frame *pptrap_frame;
    struct trap_frame *pctrap_frame;
    uint64_t child_pid;

    daif = local_irq_disable_save();
    
    // initialize mm
	child->mm = mm_struct_create();
	// copy vma in parent->mm to child->mm except for user stack1
	dup_mm_struct(child->mm, parent->mm);
	// create user stack vma
	//dup_vma_stack(child->mm, parent->mm);
	//create vc ram identity mapping
//	create_vma_vc(child->mm);


    // initialize child's trap frame and kernel stack
	child->stack = alloc_pages(1);
    memcpy(child->stack, parent->stack, PAGE_SIZE * 2);

    pptrap_frame = get_trap_frame(parent);
    pctrap_frame = get_trap_frame(child);
    LOG("parent kernel stack: %p", parent->stack + PAGE_SIZE * 2);
    LOG("parent trap frame: %p", pptrap_frame);
    LOG("child trap stack: %p", child->stack + PAGE_SIZE * 2);
    LOG("child trap frame: %p", pctrap_frame);

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
    memcpy(&child->sighandler, &parent->sighandler, sizeof(struct sighand_struct));
    
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
    struct vm_area_struct *vma;
    struct list_head* node;
    struct task_struct* child;
    
    list_del(&task->list);

    // free kernel stack
    free_pages(task->stack, 1);

    // free mm
    mm_struct_destroy(task->mm);

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
    }else{
		if(task->child){
			list_for_each(node, &task->child->siblings){
                child = list_entry(node, struct task_struct, siblings);
                child->parent = task->child;
            }
            task->child->child = list_first_entry(&task->child->siblings, struct task_struct, siblings);
            list_del(&task->child->siblings);
			task->child->parent = NULL;
		}
	}
    // free fs
    kfree(task->files); // remember to close all files
    kfree(task->fs);

    // free signal pending queue
    free_sigpendings(&task->sigpending);
    kfree(task);
    if(user_init == task) user_init = NULL;
}

int task_exec(const char* name, char* const argv[]){
    LOG("enter task_exec");
    LOG("file name: %s", name);
    int ret = -1; 
    struct list_head* node;
    struct vm_area_struct *vma;
    struct task_struct* current = get_current();
    struct trap_frame *trap_frame = get_trap_frame(current);
    volatile uint64_t daif;
	char* tmp_name;
    size_t s;

	// since we will destroy pgb and it will cause page table broken,
	// we have to save the name in userspace first
	tmp_name = kmalloc(strlen(name) + 10);	
	strcpy(tmp_name, name);
	// initialize mm
	if(s = initrdfs_filesize((char*)tmp_name)){
		mm_struct_destroy(current->mm);
		current->mm = mm_struct_create();
		// load executable 
		// create code memory area
		vma = create_vma_code(current->mm,(char*)tmp_name);
		trap_frame->elr_el1 = (uint64_t)vma->vm_start;
		//create vc ram identity mapping
		create_vma_vc(current->mm);
		
		// create user stack
		vma = create_vma_stack(current->mm);
		trap_frame->sp_el0 = (uint64_t)vma->vm_end;
		
		// initialize Saved Program Status Register el1
        trap_frame->spsr_el1 = 0;  
        ret = 0;
	}
	kfree(tmp_name);

    // initialize signal
    default_sighand_init(&current->sighandler);
	// switch ttbr0_el1 before return to user space
    daif = local_irq_disable_save();
	switch_ttbr0(virt_to_phys(current->mm->pgd));
    local_irq_restore(daif);
    LOG("end task_exec");
    return ret;
}


void run_init_task(char* filename){
    struct vm_area_struct* vma;
    uint64_t user_entry, user_sp;
    struct trap_frame* ptrap_frame;
    volatile uint64_t daif;
    LOG("run_init_task enter");
    LOG("Size of struct trap_frame: %x", sizeof(struct trap_frame));

    daif = local_irq_disable_save();
    struct task_struct* task = (struct task_struct*)kmalloc(sizeof(struct task_struct));

    // initialize mm
    task->mm = mm_struct_create();
    // load executable 
    // create code memory area
	vma = create_vma_code(task->mm, filename);
    user_entry = (uint64_t)vma->vm_start;
	//create vc ram identity mapping
	create_vma_vc(task->mm);
	
    // create user stack
	vma = create_vma_stack(task->mm);
    user_sp = (uint64_t)vma->vm_end;

    LOG("vma: %p, user_sp: %p, user_entry: %p", vma, user_sp, user_entry);
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

    // initialize fs
    task->files = (struct files_struct*)kmalloc(sizeof(struct files_struct));
    memset(task->files, 0, sizeof(struct files_struct));
    task->fs =  (struct fs_struct*)kmalloc(sizeof(struct fs_struct));
    task->fs->root = rootfs->mnt_root;
    task->fs->pwd = rootfs->mnt_root;
    
    // insert task to run queue and task list
    user_init = task;
    list_add_tail(&task->list, &task_list);
    local_irq_restore(daif);

    LOG("run_init_task end");
    add_task_to_rq(task);
}

struct vm_area_struct* create_vma_vc(struct mm_struct* mm){
	uint64_t va, pa;
	struct vm_area_struct *vma;
	uint8_t* addr;

	vma = kmalloc(sizeof(struct vm_area_struct));
    vma->vm_start = VMA_VC_BASE;
    vma->vm_end = VMA_VC_END;
    vma->type = VMA_VC_RAM;
	vma->vm_flags = VMA_PROT_READ | VMA_PROT_WRITE;
    vma->filename == NULL;
    list_add_tail(&vma->list, &mm->mmap_list);
/*
	va = vma->vm_start;
	while(va != vma->vm_end){
		// identity mapping for vc ram
		mappages(mm->pgd, va, va, PAGE_SIZE, VM_PTE_USER_ATTR);
		va += PAGE_SIZE;
	}*/
	return vma; 
}

struct vm_area_struct* create_vma(struct mm_struct* mm, uint64_t vm_start, uint64_t vm_end, uint64_t prot, uint64_t type){
	uint64_t va, pa;
	struct vm_area_struct *vma;
	uint8_t* addr;

	vma = kmalloc(sizeof(struct vm_area_struct));
    vma->vm_start = vm_start;
    vma->vm_end = vm_end;
    vma->type = type;
	vma->vm_flags = prot;
    vma->filename == NULL;
    list_add_tail(&vma->list, &mm->mmap_list);

	return vma; 
}
struct vm_area_struct* create_vma_stack(struct mm_struct* mm){
	uint64_t va, pa;
	struct vm_area_struct *vma;
	uint8_t* addr;

	vma = kmalloc(sizeof(struct vm_area_struct));
    vma->vm_start = VMA_STACK_END - VMA_STACK_SIZE;
    vma->vm_end = VMA_STACK_END;
    vma->type = VMA_STACK;
	vma->vm_flags = VMA_PROT_READ | VMA_PROT_WRITE;
    vma->filename == NULL;
    list_add_tail(&vma->list, &mm->mmap_list);

	va = vma->vm_start;
    /*
	while(va != vma->vm_end){
		addr = calloc_page();
		mappages(mm->pgd, va, virt_to_phys(addr), PAGE_SIZE, VM_PTE_USER_ATTR);
		
		va += PAGE_SIZE;
	}*/
	return vma; 

}

struct vm_area_struct* create_vma_code(struct mm_struct* mm, char* filename){
	uint64_t va, pa;
	size_t size, offset = 0, n;
	struct vm_area_struct *vma;
	uint8_t* addr;
    char* tmp_name;

    vma = kmalloc(sizeof(struct vm_area_struct));
    size = initrdfs_filesize(filename);
    vma->vm_start = VMA_CODE_BASE;
    vma->vm_end = VMA_CODE_BASE + ALIGN_UP(size, PAGE_SIZE);
	vma->vm_flags = VMA_PROT_READ | VMA_PROT_WRITE | VMA_PROT_EXEC;
    vma->type = VMA_FILE;
    tmp_name = kmalloc(strlen(filename) + 5);	
	strcpy(tmp_name, filename);

    vma->filename = tmp_name;
    list_add_tail(&vma->list, &mm->mmap_list);

    // copy file to code memory area and set page table
/*
	va = VMA_CODE_BASE;
	while(size){
		addr = calloc_page();
		n = initrdfs_loadfile(filename, addr, offset, PAGE_SIZE);
		mappages(mm->pgd, va, virt_to_phys(addr), PAGE_SIZE, VM_PTE_USER_ATTR);
		
		va += PAGE_SIZE;
		size -= n;
		offset += n;
	}
*/
	return vma; 
}

// Only duplicate area that not belong to user stack
void dup_mm_struct(struct mm_struct* dst_mm, struct mm_struct* src_mm){
	uint64_t va;
	struct list_head* node;
	struct vm_area_struct* vma,*tmp_vma;
    char* tmp_name;
	
    list_for_each(node, &src_mm->mmap_list){
        tmp_vma = list_entry(node, struct vm_area_struct, list);
        LOG("src_mm's vm area type 0x%x", tmp_vma->type);
        vma = (struct vm_area_struct*)kmalloc(sizeof(struct vm_area_struct));    
        vma = kmalloc(sizeof(struct vm_area_struct));
        vma->vm_start = tmp_vma->vm_start;
        vma->vm_flags = tmp_vma->vm_flags;
        vma->vm_end = tmp_vma->vm_end;
        vma->type = tmp_vma->type;
        if(tmp_vma->type == VMA_FILE){
            tmp_name = kmalloc(strlen(tmp_vma->filename) + 5);	
	        strcpy(tmp_name, tmp_vma->filename);
            vma->filename = tmp_name;
        }

        // VC RAM should do copy on write and it only support demand paging
        if(vma->type != VMA_VC_RAM){
            // COW
            dup_pages(dst_mm->pgd, src_mm->pgd, vma->vm_start, vma->vm_end - vma->vm_start, PAGE_ATTR_RDONLY);
            // also set parent's page to read only to trigger COW
            mappages(src_mm->pgd, tmp_vma->vm_start, 0x0, tmp_vma->vm_end - tmp_vma->vm_start, PAGE_ATTR_RDONLY, 1);
        }

        list_add_tail(&vma->list, &dst_mm->mmap_list);
    }
}

struct mm_struct* mm_struct_create(){
	struct mm_struct *mm = kmalloc(sizeof(struct mm_struct));
    INIT_LIST_HEAD(&mm->mmap_list);
	mm->pgd = calloc_page();
	return mm;
}

void mm_struct_destroy(struct mm_struct* mm){
	struct vm_area_struct *vma;
    while(!list_empty(&mm->mmap_list)){
        vma = list_first_entry(&mm->mmap_list, struct vm_area_struct, list);
        if(vma->type == VMA_FILE){
            kfree(vma->filename);
        }
        list_del(&vma->list); 
		kfree(vma);
    }
	free_page_table(mm->pgd);
    kfree(mm); 
}

struct vm_area_struct* find_vma(struct mm_struct *mm, uint64_t addr){
    struct vm_area_struct *vma;
    struct list_head *node;
    list_for_each(node, &mm->mmap_list){
        vma = list_entry(node, struct vm_area_struct, list);
        if(vma->vm_start <= addr && vma->vm_end > addr){
            return vma;
        }
    }
    return NULL;
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

uint64_t mmap(void* addr, size_t len, int prot, int flags, int fd, int file_offset){
    struct task_struct* current = get_current();
    struct mm_struct* mm = current->mm;
    struct vm_area_struct* tmp_vma, *vma;
    uint64_t vm_start = 0, vm_end;
    uint64_t daif;
    uint64_t tmp_prot = 0, tmp_type = 0;
    
    INFO("mmap(%p, %p, %p, %p, %p, %p)", addr, len, prot, flags, fd, file_offset);
    tmp_prot = prot & (VMA_PROT_READ | VMA_PROT_WRITE | VMA_PROT_EXEC);
    if(flags & MAP_ANONYMOUS) tmp_type = VMA_ANONYMOUS;
    
    daif = local_irq_disable_save(); 
    if(addr != NULL){
        vm_start = ALIGN_DOWN(addr, PAGE_SIZE);
    }else{
        vm_start =  0x0;
    }
    
    while(tmp_vma = find_vma(mm , vm_start)){
         vm_start = tmp_vma->vm_end;
    }
    vm_end = vm_start + ALIGN_UP(len, PAGE_SIZE);
    create_vma(mm, vm_start, vm_end, tmp_prot, tmp_type);
    
    INFO("return value: %p", vm_start);
    local_irq_restore(daif); 
    return vm_start;
}

void* sys_mmap(void* addr, size_t len, int prot, int flags, int fd, int file_offset){
    return (void*)mmap(addr, len, prot, flags, fd, file_offset);
}

