#include <types.h>
#include <sched.h>
#include <timer.h>
#include <current.h>
#include <list.h>

#define SCHEDULER_TIMER_HZ 1000
#define SCHEDULER_WATERMARK 15

static struct list_head run_queue;

static uint32 schedule_ticks;

static void timer_schdule_tick(void *_)
{
    schedule_tick();

    timer_add_proc_freq(timer_schdule_tick, NULL, SCHEDULER_TIMER_HZ);
}

void scheduler_init(void)
{
    INIT_LIST_HEAD(&run_queue);

    timer_add_proc_freq(timer_schdule_tick, NULL, SCHEDULER_TIMER_HZ);
}

void schedule(void)
{
    uint64 daif;
    task_struct *task;

    daif = save_and_disable_interrupt();

    task = list_first_entry(&run_queue, task_struct, list);

    list_del(&task->list);
    list_add_tail(&task->list, &run_queue);

    current->need_resched = 0;

    restore_interrupt(daif);

    switch_to(current, task);
}

void schedule_tick(void)
{
    schedule_ticks += 1;

    if (schedule_ticks >= SCHEDULER_WATERMARK) {
        schedule_ticks = 0;

        current->need_resched = 1;
    }
}

void sched_add_task(task_struct *task)
{
    uint32 daif;

    daif = save_and_disable_interrupt();

    list_add_tail(&task->list, &run_queue);

    restore_interrupt(daif);
}