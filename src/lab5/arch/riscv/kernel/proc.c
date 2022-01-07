#include "proc.h"
#include "defs.h"
#include "mm.h"
#include "rand.h"
#include "printk.h"
#include "vm.h"

extern void __dummy();
extern void __switch_to(struct task_struct* prev, struct task_struct* next);
extern char uapp_start[];
extern char uapp_end[];

struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组，所有的线程都保存在此

const unsigned long OffsetOfThreadInTask = (unsigned long)OFFSET(struct task_struct, thread);
const unsigned long OffsetOfRaInTask = OffsetOfThreadInTask+(unsigned long)OFFSET(struct thread_struct, ra);
const unsigned long OffsetOfSpInTask = OffsetOfThreadInTask+(unsigned long)OFFSET(struct thread_struct, sp);
const unsigned long OffsetOfSInTask = OffsetOfThreadInTask+(unsigned long)OFFSET(struct thread_struct, s);
const unsigned long OffsetOfSepcInTask = OffsetOfThreadInTask+(unsigned long)OFFSET(struct thread_struct, sepc);

void task_init() {
    // printk("...proc_init start!\n");
    // 调用 kalloc() 为 idle 分配一个物理页
    idle = (struct task_struct*)kalloc();

    // 设置 state 为 TASK_RUNNING;
    idle->state = TASK_RUNNING;

    // 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    idle->counter = 0;
    idle->priority = 0;

    // 设置 idle 的 pid 为 0
    idle->pid = 0;

    // 将 current 和 task[0] 指向 idle
    current = idle;
    task[0] = idle;

    // 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    for (int i = 1; i < NR_TASKS; i++) {
        task[i] = (struct task_struct*)kalloc();
        // 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
        task[i]->state = TASK_RUNNING;
        task[i]->counter = 0;
        task[i]->priority = rand()%(PRIORITY_MAX-PRIORITY_MIN+1)+PRIORITY_MIN;
        task[i]->pid = i;
        // 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`, 其中 `ra` 设置为 __dummy （见 4.3.2）的地址， `sp` 设置为 该线程申请的物理页的高地址
        task[i]->thread.ra = (unsigned long)__dummy;
        task[i]->thread.sp = (unsigned long)(task[i])+PGSIZE;

        // 通过 kalloc 接口申请一个空的页面来作为 U-Mode Stack
        task[i]->kernel_sp = (unsigned long)(task[i])+PGSIZE;
        task[i]->user_sp = kalloc();

        pagetable_t pgtbl = (pagetable_t)kalloc();
        memcpy(pgtbl, swapper_pg_dir, PGSIZE);

        unsigned long va = USER_START;
        unsigned long pa = (unsigned long)(uapp_start)-PA2VA_OFFSET;
        create_mapping(pgtbl, va, pa, (unsigned long)(uapp_end)-(unsigned long)(uapp_start), 31);

        va = USER_END-PGSIZE;
        pa = task[i]->user_sp-PA2VA_OFFSET;
        create_mapping(pgtbl, va, pa, PGSIZE, 23);
        
        unsigned long satp = csr_read(satp);
        satp = (satp >> 44) << 44;
        satp |= ((unsigned long)pgtbl-PA2VA_OFFSET) >> 12;
        task[i]->pgd = satp;

        task[i]->thread.sepc = USER_START;
        unsigned long sstatus = csr_read(sstatus);
        sstatus &= ~(1<<8); // set sstatus[SPP] = 0
        sstatus |= 1<<5; // set sstatus[SPIE] = 1
        sstatus |= 1<<18; // set sstatus[SUM] = 1
        task[i]->thread.sstatus = sstatus;
        task[i]->thread.sscratch = USER_END;
    }

    // printk("OffsetOfRaInTask = %d\n", OffsetOfRaInTask);
    // printk("OffsetOfSpInTask = %d\n", OffsetOfSpInTask);
    // printk("OffsetOfSInTask = %d\n", OffsetOfSInTask);
    // printk("OffsetOfSepcInTask = %d\n", OffsetOfSepcInTask);
    printk("...proc_init done!\n");
}

void dummy() {
    unsigned long MOD = 1000000007;
    unsigned long auto_inc_local_var = 0;
    int last_counter = -1;
    while(1) {
        if (last_counter == -1 || current->counter != last_counter) {
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            // printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
            printk("[PID = %d] is running. thread space begin at 0x%lx\n", current->pid, (unsigned long)current);
        }
    }
}

void switch_to(struct task_struct* next) {
    if (current != next) {
    #ifdef SJF
        printk("\n");
        printk("switch to [PID = %d COUNTER = %d]\n", next->pid, next->counter);
    #endif 

    #ifdef PRIORITY
        printk("\n");
        printk("switch to [PID = %d PRIORITY = %d COUNTER = %d]\n", next->pid, next->priority, next->counter);
    #endif

        struct task_struct* prev = current;
        current = next;
        __switch_to(prev, next);
    }
    else return;
}

void do_timer(void) {
    /* 如果当前线程是 idle 线程 直接进行调度 */
    if (current == idle) {
        schedule();
    }
    /* 如果当前线程不是 idle 对当前线程的运行剩余时间减 1 
       若剩余时间仍然大于0 则直接返回 否则进行调度 */
    else {
        current->counter--;
        if (current->counter > 0) return;
        else schedule();
    }
}

void schedule() {
#ifdef SJF
    SJF_schedule();
#endif

#ifdef PRIORITY
    Priority_schedule();
#endif
}

void SJF_schedule() {
    struct task_struct* next = current;
    unsigned long min_counter = COUNTER_MAX+1;
    while (1) {
        for (int i = NR_TASKS; i > 0; i--) {
            if (task[i]->counter == 0) continue;
            if (task[i]->counter < min_counter) {
                min_counter = task[i]->counter;
                next = task[i];
            }
        }

        if (min_counter == COUNTER_MAX+1) {
            for (int i = 1; i < NR_TASKS; i++) {
                task[i]->counter = rand()%(COUNTER_MAX-COUNTER_MIN+1)+COUNTER_MIN;
                // printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);
            }
            printk("\n");
        }
        else break;
    }

    switch_to(next);
}

void Priority_schedule() {
    struct task_struct* next = current;
    unsigned long max_priority = PRIORITY_MIN-1;
    while (1) {
        for (int i = 1; i < NR_TASKS; i++) {
            if (task[i]->counter == 0) continue;
            if (task[i]->priority > max_priority) {
                max_priority = task[i]->priority;
                next = task[i];
            }
        }

        if (max_priority == PRIORITY_MIN-1) {
            for (int i = 1; i < NR_TASKS; i++) {
                task[i]->counter = rand()%(COUNTER_MAX-COUNTER_MIN+1)+COUNTER_MIN;
                printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", task[i]->pid, task[i]->priority, task[i]->counter);
            }
        }
        else break;
    }

    switch_to(next);
}