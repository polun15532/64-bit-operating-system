#include "schedule.h"
#include "lib.h"
#include "task.h"
#include "timer.h"

void insert_task_queue(struct task_struct *tsk)
{
    if (tsk == &init_task_union.task)
        return;
    struct task_struct *tmp = container_of(list_next(&task_schedule.task_queue.list), struct task_struct, list);
    
    if (!list_is_empty(&task_schedule.task_queue.list)) {
        while (tmp->vrun_time < tsk->vrun_time) {
            tmp = container_of(list_next(&task_schedule.task_queue.list), struct task_struct, list);
        }
    }
    list_add_to_before(&tmp->list, &tsk->list);
    task_schedule.running_task_count++;
}

// 從ready queue選取優先最高的任務，並從佇列中刪除他
struct task_struct *get_next_task()
{
    struct task_struct *tsk = NULL;
    if (list_is_empty(&task_schedule.task_queue.list))
        return &init_task_union.task; // 所有任務都執行完
    tsk = container_of(list_next(&task_schedule.task_queue.list), struct task_struct, list);
    list_del(&tsk->list);
    task_schedule.running_task_count -= 1;
    return tsk;
}

void schedule_init()
{
    memset(&task_schedule, 0, sizeof(task_schedule));
    list_init(&task_schedule.task_queue.list);
    task_schedule.task_queue.vrun_time = 0x7fffffffffffffff;
    task_schedule.running_task_count = 1;
    task_schedule.CPU_exec_task_jiffies = 4;
}

void schedule()
{
    struct task_struct *tsk = get_next_task(); // 從佇列中取得下一任務。
    current->flags &= ~NEED_SCHEDULE;
    if (current->vrun_time >= tsk->vrun_time) {
        if (current->state == TASK_RUNNING)
            insert_task_queue(current);
        if (!task_schedule.CPU_exec_task_jiffies) {
            // 重新分配新時間片，這裡假設分配任務數量任務小於等於4，將4個時間片平均分配到任務上。
            switch (tsk->priority) {
                case 0:
                case 1:
                    task_schedule.CPU_exec_task_jiffies = 4 / task_schedule.running_task_count;
                    break;
                case 2:
                default:
                    task_schedule.CPU_exec_task_jiffies = 4 / task_schedule.running_task_count * 3;
                    break;                    
            }
        }
        switch_to(current, tsk); // 切換任務
    } else {
        insert_task_queue(tsk); // current執行的緊迫性比tsk高，將tsk放回任務佇列。
        if (!task_schedule.CPU_exec_task_jiffies) {
            switch (tsk->priority) {
                case 0:
                case 1:
                    task_schedule.CPU_exec_task_jiffies = 4 / task_schedule.running_task_count;
                    break;
                case 2:
                default:
                    task_schedule.CPU_exec_task_jiffies = 4 / task_schedule.running_task_count * 3;
                    break;                    
            }
        }
    }
}