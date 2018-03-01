/***** kill a process *****/

#include <sys/signal.h>

int sys_kill(pid_t pid, int sig)
{
  /* determine the pid and act accordingly */
  struct pcb *current = get_current_process();  /* let us not kill current, change of mind : will kill it */
  if(pid < 0)
  {
    return 0;  // no process group in 506
  }

  else if(pid == 0 || pid == 1)
  {
    kprintf("cannot kill swapper and idle process in BLEH\n");
    return 0;
  }
  /* if current process then immediately exit from here */
  else if(current->pid == pid)
  {
    kprintf("kill -9 <current> is about to kill current process\n");
    return 9;
  }
  else return send_signal(pid, sig);
}

int send_signal(pid_t pid, int sig)
{
  //kprintf("signal %d ", pid);
  struct pcb *proc = get_process_descriptor_from_pid(pid);
  if(proc == NULL)
  {
    goto end;
  }
  if(sig == 9)    // kill -9 
    proc->is_sigkill_pending = 1;  // this flag checked when process is first scheduled
  else if(sig == 11)   // segmentation fault
    proc->is_sigsegv_pending = 1;
  
  if(proc->on_rq == 0)
  {
    if(proc->thread_state == SLEEPING && proc->process_state == NORMAL)
    {
      try_to_wake_up(&sleeping_queue, proc);
    }
    else if(proc->thread_state == BLOCKED && proc->process_state == NORMAL)
    {
      try_to_wake_up(&blocked_queue, proc);
    }
    else if(proc->thread_state == WAITING && proc->process_state == NORMAL)
    {
      try_to_wake_up(&waiting_queue, proc);
    }
    else if(proc->thread_state == STOPPED)
    {
      kprintf("Process is already dead\n");
      goto end;
    }
    schedule();
  }
  
  end:
    return 1;
}


