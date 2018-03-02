/******  current_task -> time_slice  ******/

#ifndef  __SCHEDULER_H
#define  __SCHEDULER_H

#include <sys/defs.h>
#include <sys/process.h>
#include <sys/queue.h>
#include <sys/gdt.h>
#include <sys/pit.h>
#include <sys/queue.h>
#include <sys/util.h>
#include <sys/syscalls.h>

#define TIME_SLICE     1000

struct mm_struct;   // c forward declarations

struct queue_type;

extern int is_blocked_condition_true ;

extern int ticks;

/* addition of prev has been forced by the introduction of waitqueues */

struct pcb *current;

void scheduler_init();

void init_idle();

void context_switch(struct pcb *, struct pcb *);

void switch_mm(struct mm_struct *, struct mm_struct *);

void switch_to(struct pcb *, struct pcb *);

void post_stack_switch(struct pcb *, struct pcb *);

void block_process(struct pcb *);

void try_to_wake_up(struct queue_type *, struct pcb *);

void do_switch(struct pcb *prev, struct pcb *next);

int sys_sleep_process(unsigned long );

void schedule();

void cpu_idle();

struct pcb *pick_next_task(struct pcb *);

struct pcb *get_current_process();
//void schedule_timeout();

#endif
