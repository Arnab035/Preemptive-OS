//******************   Preemptive round-robin scheduler   ********************/

#include "sys/scheduler.h"

int is_blocked_condition_true=0;

void scheduler_init()
{
  // setup all the relevant queues
  //kprintf("Setting up all the relevant queues\n");
  init_scheduler_queues();
  init_idle();   /* init idle process */
}

void init_idle()
{
  // allocate memory for idle_task
  idle_task = (struct pcb *)phys_mem_alloc();
  idle_task = (struct pcb *)((uint64_t)idle_task | KERNBASE);

  idle_task->on_rq = 0;  // not in runqueue
  idle_task->mm = init_mm();

  idle_task->kstack = (uint64_t *)phys_mem_alloc();
  idle_task->kstack = (uint64_t *)((uint64_t)idle_task->kstack | KERNBASE);

  my_memset((void *)idle_task->kstack, 0, PAGE_BLOCK_SIZE);   // all register values to be zero

  idle_task->flags = PF_KTHREAD;
  idle_task->esp0 = (uint64_t)((uint64_t)idle_task->kstack + PAGE_BLOCK_SIZE);   // top of kstack

  idle_task->is_sigkill_pending = 0; // no pending signals
  idle_task->is_sigsegv_pending = 0;

  idle_task->command = (char *)phys_mem_alloc();
  idle_task->command = (char *)((uint64_t)idle_task->command | KERNBASE);

  my_memset((void *)idle_task->command, 0, PAGE_BLOCK_SIZE);

  my_memcpy((void *)idle_task->command, "Idle Task", my_strlen("Idle Task") + 1);

  idle_task->pid = 0;
  idle_task->process_state = NORMAL;
  idle_task->thread_state = RUNNING;

  idle_task->time_slice = TIME_SLICE;
  //idle_task->foregorund = 0; /* idle task foreground - it does not connect to terminal ? */
  idle_task->pptr = idle_task;
  idle_task->mm->rip = (uint64_t)cpu_idle;
  current = idle_task;
  //insert_allproc(idle_task);   /* idle task always remains in the allproc queue- until you shutdown the system */
  //add_process_to_queue(&run_queue, idle_task);  // this ensures atleast one process remains in the runqueue
}

void context_switch(struct pcb *prev, struct pcb *next)
{
  struct mm_struct *prev_mm, *next_mm;
  prev_mm = prev->mm;
  next_mm = next->mm;
  
  switch_mm(prev_mm, next_mm);
  switch_to(prev, next);
}

void switch_mm(struct mm_struct *prev_mm, struct mm_struct *next_mm)
{
  set_pml4(next_mm->cr3);  // flush tlb
}

void switch_to(struct pcb *prev, struct pcb *next)
{
  do_switch(prev, next);
}

void post_stack_switch(struct pcb *prev, struct pcb *next)
{
  current = next;
  set_tss_rsp((void *)next->esp0);
  // update the global kernel data structure that we use in swapgs
  global_ds->k_rsp = (uint64_t)next->esp0;
  global_ds->u_rsp = 0;
  uint64_t gs_kernel_base = (uint64_t)global_ds;
  uint32_t lo_gs_addr = gs_kernel_base;
  uint32_t hi_gs_addr = gs_kernel_base>>32;

  do_wrmsr(MSR_GS_KERNEL_BASE, lo_gs_addr, hi_gs_addr);
}

void block_process(struct pcb *process)
{
  if(!process) return;
  process->process_state = NORMAL;
  process->thread_state = BLOCKED;
  schedule();  /* schedule another task -- handle dequeueing */
}

/* Wakeups don't really cause entry into schedule(). They add a
 *      task to the run-queue and that's it. */

void try_to_wake_up(struct queue_type *queue, struct pcb *process)
{
  if(!process) return;
  process->process_state = NORMAL;
  process->thread_state = RUNNING;

  remove_process_from_queue(queue, process);
  if(process->on_rq == 0)
  {
    add_process_to_queue_front(&run_queue, process);  // you want the woken up process to run first, not later
    process->on_rq = 1;
  }
}
/* schedule() is invoked in two ways - there is a direct invocation and a lazy invocation. Lazy involves timers
   Direct invocation is direct call of schedule() during wait/exit/fork etc. */

void schedule()
{
  struct pcb *prev = get_current_process();
  struct pcb *next;
  if(prev->time_slice <= 0)
  {
    prev->time_slice = TIME_SLICE;
  }
  //kprintf("Finding next process to schedule...\n");
  if(prev->process_state == ZOMBIE && prev->thread_state == STOPPED)
  {
    /* remove zombie processes */
    if(prev->on_rq == 1)
    {
      remove_process_from_queue(&run_queue, prev);
    }
    prev->on_rq = 0;
  }
  else if(prev->process_state == NORMAL && prev->thread_state == SLEEPING)
  {
      // remove sleeping process from the runqueue and push it to waitqueue
    if(prev->on_rq == 1)
    {
      remove_process_from_queue(&run_queue, prev);
      prev->on_rq = 0;
      add_process_to_queue(&sleeping_queue, prev);
    }
  }
  else if(prev->process_state == NORMAL && prev->thread_state == BLOCKED)
  {
    /* dequeue task */
    if(prev->on_rq == 1)
    {
      remove_process_from_queue(&run_queue, prev);
      prev->on_rq = 0;
      add_process_to_queue(&blocked_queue, prev);
    }
  }
  else if(prev->process_state == NORMAL && prev->thread_state == WAITING)
  {
    /* handle process calling wait */
    if(prev->on_rq == 1)
    {
      remove_process_from_queue(&run_queue, prev);
      prev->on_rq = 0;
      add_process_to_queue(&waiting_queue, prev);
    }
  }


  next = pick_next_task(prev);
  if(next == NULL && (prev->process_state == NORMAL && prev->thread_state == RUNNING))
  {
    next = prev;  /* do not change current */
  }
  
  if(next == NULL)
  {
    /* this is the idle thread -- do not do anything and try to conserve power */
    current = idle_task;
    context_switch(prev, current);
    //cpu_idle();
  }
  else if(prev != next)
  {
    current = next;       // set current process
    if(next->sleep_time <= 0)
    {
      next->sleep_time = 0;
    }
    context_switch(prev, next);
  }
}

/* wait for somebody to say they want to schedule */

void cpu_idle()
{
  while(1);
}

struct pcb *pick_next_task(struct pcb *curr)
{
  struct pcb *next;
  next = select_first_process_from_queue(&run_queue);
  return next;
}

struct pcb *get_current_process()
{
  return current;
}


/* this is the implementation of sleep_2 system call- assign a different number to it */
int sys_sleep_process(unsigned long seconds)
{
  if(seconds < 1)
  {
    kprintf("usage sleep <seconds> : where seconds is positive\n");
    return -1;
  }
  struct pcb *curr = get_current_process();
  curr->process_state = NORMAL;
  curr->thread_state  = SLEEPING;
  curr->sleep_time = seconds * 1000;   //(in ms)
  
  kprintf("Process with pid %d going to sleep..\n", curr->pid);
 
  //cpu_idle_with_timeout(seconds); 
  schedule();
  //kprintf("remaining timeout : %d\n", timeout);
  kprintf("Waking up from sleep...\n");  
  return 0;
}

