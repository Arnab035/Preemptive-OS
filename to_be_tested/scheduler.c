/******************   Preemptive round-robin scheduler   ********************/

#include "sys/scheduler.h"

void schedule()
{
  struct pcb *curr = get_current_process();
  assert(curr != NULL);
  struct pcb *next;
  struct pcb *prev;
  if(curr->time_slice <= 0)
  {
    // switch task
    curr->time_slice = 10;   // assumed 10 ticks
    kprintf("Process exhausted its time slice --- continuing with new process\n");
    next = get_next_process_to_schedule();
    if(next != NULL)
    {
      current = next;       // set current process
      // flush TLBs for new process
      set_pml4(next->cr3);
      prev    = remove_process_from_runqueue();
      insert_process_into_runqueue(prev);
    }
    // if no next process let current be the same  
  }
  else return;
}

// just use round-robin strategy to obtain the next process from the queue
struct pcb *get_next_process_to_schedule()
{ 
  // head of runqueue points to the process being scheduled
  struct pcb *curr = get_current_process();
  return ((curr->next != NULL) ? curr->next : NULL);
}

struct pcb *get_current_process()
{
  return current;
}

void insert_process_into_runqueue(struct pcb *process)
{
  /* handle the insertion of the first process into runqueue */
  if(!head_of_runqueue)
  {
    head_of_runqueue = process;
    current = process;
    return;
  }
  struct pcb *head = head_of_runqueue;
  
  while(head->next != NULL)
  {
    head = head->next;
  }
  head->next = process;  // insert process at the end of the queue
  process->next = NULL;
  return;
}

// removes the head of the runqueue
struct pcb *remove_process_from_runqueue()
{
  if(!head_of_runqueue)
  {
    kprintf("No processes to remove from runqueue\n");
    return NULL;
  }
  struct pcb *temp = head_of_runqueue;
  head_of_runqueue = head_of_runqueue->next;
  if(head_of_runqueue == NULL)
  {
    kprintf("runqueue has only one process and is empty after removal\n");
  }
  return temp;
} 
