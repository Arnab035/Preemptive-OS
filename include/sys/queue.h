#ifndef __QUEUE_H
#define __QUEUE_H

#include <sys/defs.h>
#include <sys/process.h>

struct queue_type
{
  struct pcb *processes;
  int flags;       // may be used
};

struct queue_type run_queue;

struct queue_type blocked_queue;

struct queue_type sleeping_queue;

struct queue_type waiting_queue;

struct pcb *select_first_process_from_queue(struct queue_type *);

void add_process_to_queue(struct queue_type *, struct pcb *);

void add_process_to_queue_front(struct queue_type *, struct pcb *);

void remove_process_from_queue(struct queue_type *, struct pcb *);

struct pcb *remove_first_process_from_queue(struct queue_type *);

void init_scheduler_queues();

#endif
