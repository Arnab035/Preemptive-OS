#ifndef __START_H
#define __START_H

#include <sys/defs.h>
#include <sys/mmgr.h>
#include <sys/kprintf.h>
#include <sys/syscalls.h>
#include <sys/idt.h>
#include <sys/process.h>
#include <sys/tarfs.h>
#include <sys/types.h>

/*
struct mm_struct 
{
  .vma = NULL,
  .exec = 0,
  .file = NULL,
  .rip = 0;
  .cr3 = pml4;  // from virt_mmgr.c
}init_mm;
*/

struct pcb *idle_task;

//struct mm_struct init_mm;

void start_kernel();

void final_init();

int kernel_init();

int run_init_process(const char *);

void kernel_thread(int (*fn)(), void *arg);

#endif
