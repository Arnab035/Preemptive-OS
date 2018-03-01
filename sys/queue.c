/************  queue based module ************/

/*************  only contains standard queueing operations that we will/may use in our scheduler ************/

/*** this is a round-robin scheduler hence all tasks will be added to the end of the queue ***/

#include <sys/queue.h>

void init_scheduler_queues()
{
  run_queue.processes = NULL;
  run_queue.flags = 0;

  blocked_queue.processes = NULL;
  blocked_queue.flags = 0;
  
  sleeping_queue.processes = NULL;
  sleeping_queue.flags = 0;

  waiting_queue.processes = NULL;
  waiting_queue.flags = 0;
}

void add_process_to_queue_front(struct queue_type *queue, struct pcb *curr)
{
  /* teh start of teh queue */
  if(queue->processes == NULL)
  {
    queue->processes = curr;
    curr->next_in_queue = NULL;
  }
  else
  {
    struct pcb *processes = queue->processes;
    queue->processes = curr;
    curr->next_in_queue = processes;
  }
}

void add_process_to_queue(struct queue_type *queue, struct pcb *curr)
{
  /* the start of the queue */
  if(queue->processes == NULL)
  {
    queue->processes = curr;
    curr->next_in_queue = NULL;
  }
  else
  {
    struct pcb *processes = queue->processes; // first in queue
    while(processes->next_in_queue != NULL)
    {
      processes = processes->next_in_queue;  
    }
    processes->next_in_queue = curr;
    curr->next_in_queue = NULL;
  }
  return;
}

/** remove a given task from the queue **/

void remove_process_from_queue(struct queue_type *queue, struct pcb *curr)
{
  if(queue->processes == NULL)
    return;

  //struct pcb *processes = queue->processes;
  /* if the head of queue */
  if(queue->processes->pid == curr->pid)
  {
    queue->processes = queue->processes->next_in_queue;
    return;
  }
  /* otherwise somewhere in queue */
  while(queue->processes->next_in_queue != NULL)
  {
    if(queue->processes->next_in_queue->pid == curr->pid)
    {
      queue->processes->next_in_queue = queue->processes->next_in_queue->next_in_queue;
      return;
    }
  }
}

/* pick next task should use the below function instead of the below remove function */

struct pcb *select_first_process_from_queue(struct queue_type *queue)
{
  if(queue->processes == NULL)
    return NULL;
 
  struct pcb *processes = queue->processes;
  
  return processes;  
}

/** remove the first processes from the queue  **/

struct pcb *remove_first_process_from_queue(struct queue_type *queue)
{
  if(queue->processes == NULL)  // queue empty
    return NULL;

  struct pcb *processes = queue->processes;
  
  remove_process_from_queue(queue, processes);
  return processes;
}

