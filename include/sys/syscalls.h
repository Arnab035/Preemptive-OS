#ifndef SYSCALL_H
#define SYSCALL_H

#include <sys/defs.h>
#include <sys/util.h>
#include <sys/fs.h>
#include <sys/vfs.h>
#include <sys/tarfs.h>
#include <sys/pit.h>

/* ideas for syscall design from http://www.jamesmolloy.co.uk/tutorial_html/10.-User%20Mode.html  */

#define MSR_STAR    0xC0000081
#define MSR_LSTAR   0xC0000082
#define IA32_EFER   0xC0000080
#define MSR_GS_KERNEL_BASE   0xC0000102

#define MAX_SYSCALLS  100

void *sys_call_table[MAX_SYSCALLS];

struct kernel_ds
{
  uint64_t k_rsp;
  uint64_t u_rsp;
};      // per cpu area to reserve stack

struct kernel_ds *global_ds;   

void init_syscall();

uint16_t syscall_max;

void syscall_handler();

void do_syscalls();

#endif
