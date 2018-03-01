#ifndef __PROCESS_H
#define __PROCESS_H

#include <sys/defs.h>
#include <sys/mmgr.h>
#include <sys/kprintf.h>
#include <sys/phys_mmgr.h>
#include <sys/scheduler.h>
#include <sys/vfs.h>
#include <sys/elf64.h>
#include <sys/memory.h>
#include <sys/fs.h>
#include <sys/signal.h>

#define PID_MAX   32768
#define MAX_FD    256

/* copied from linux */
#define CLONE_VM     0x00000100
#define CLONE_FS     0x00000200
#define CLONE_FILES  0x00000400

#define STACK_START  0x00007ffffffff000ULL
#define STACK_LIMIT  (STACK_START - 4 * PAGE_BLOCK_SIZE)


struct reg_list
{
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t rbp;
  uint64_t rbx;
  /* callee-clobbered registers */
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rax;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  /* on syscall number -> this is syscal number, on cpu exception, error code, on hw interrupt, irq number */
  uint64_t orig_rax;
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t sp;  // user space sp
  uint64_t ss;
};

struct scheduling_info
{
  uint8_t    priority;
  uint16_t   user_mode_priority;
  uint64_t   sleeping;
};

/* this is the swapped out process frame */

/* will be applicable when we do switch_to(prev, next) */

/* process->rsp points to this */
struct inactive_process_frame
{
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t rbx;
  uint64_t rbp;
  uint64_t ret_addr;  // ret_addr of fork 
};

struct fork_frame
{
  struct inactive_process_frame old_frame;
  struct reg_list regs;
};

struct vm_map_entry
{
  uint64_t start_addr;
  uint64_t end_addr;
  uint64_t filesize;
  uint64_t memsize;
  uint64_t fileoffset;
  uint64_t protection;     // protection flags
  struct   file *file;
  struct   vm_map_entry *next;
  struct   vm_map_entry *prev;
  int (*fault)(struct mm_struct *, struct vm_map_entry *, uint64_t );   // page fault handler
};

struct mm_struct
{
  struct vm_map_entry *vma;
  uint64_t exec;    // top of stack
  struct file *file;
  uint64_t rip;   // entry point
  uint64_t cr3; 
};

/* hack : we are dealing with only 2 signals here - either sigkill/sigsegv - so put separatepending signal indicators inside the process descriptor : make sure process desc is < 1024 */

struct pcb
{
  uint64_t esp0;
  uint64_t rsp;
  uint64_t *kstack;
  enum     {PF_UTHREAD, PF_KTHREAD} flags; 
  uint8_t  foreground;   // is a foreground/background task
  uint8_t  is_sigkill_pending;
  uint8_t  is_sigsegv_pending;
  uint16_t pid; 
  int      process_exit_status;
  uint8_t  on_rq;            // gets set to 1, when process enters runqueue
  uint64_t time_slice;
  char     *cwd;        // current-workding-dir
  uint64_t sleep_time;  // set by sleep system call on the process
  struct   pcb *pptr;
  struct   proc_list *cptr;
  char     *command;               // command - used by ps
  enum     {NEW, NORMAL, ZOMBIE, DEAD}  process_state;
  struct   pcb *next_in_queue; 
  struct   mm_struct *mm;
  struct   file *fd[MAX_FD];      // max files open
  enum     {RUNNING, BLOCKED, WAITING, SLEEPING, STOPPED}  thread_state;                    
};

struct pcb *idle_task;

struct proc_list{
  struct pcb *process;
  struct proc_list *next;
};

void ret_from_fork(void);

struct mm_struct *init_mm();

struct   proc_list *allproc, *zombproc;  //allproc is the list of all procs, zombproc is the list of zombie procs

void allproc_and_zombproc_init();

uint32_t insert_allproc(struct pcb *process);

uint32_t delete_from_allproc(struct pcb *process);

uint32_t delete_from_zombproc(struct pcb *process);

uint32_t insert_into_pchildren(struct proc_list *, struct pcb *);

void switch_to_user_cr3();

uint32_t insert_zombproc(struct pcb *);

int sys_fork();

int do_fork(int , unsigned long , unsigned long );

void increment_refcount_of_file(struct file *);

void copy_mm(int ,struct pcb *, struct pcb *);

void dup_mm(struct mm_struct *, struct mm_struct *);

struct reg_list *task_reg_list(struct pcb *);

void copy_thread(struct pcb *, struct pcb *, unsigned long, unsigned long );

int copy_vm(struct pcb *, struct pcb *);

struct file *do_open_exec(const char *, int , int );

void mm_init(struct pcb *);   // no need of stack

int count(char **);

void start_thread(struct reg_list *, uint64_t, uint64_t );

int sys_execve(const char *, const char **, const char **);

int do_execve(char *, const char **, const char **);

void sys_exit(int );

void exit_notify(struct pcb *);

int exit_files(struct file **);

int exit_vmmaps(struct pcb *, struct mm_struct *);

int sys_wait(int *);

int do_wait_thread(struct pcb *);

void release_task(struct pcb *);

int sys_getprocs64(char *, size_t);

struct pcb *get_process_descriptor_from_pid(pid_t );

char *getname(const char *);

void reparent_process_to_init(struct pcb *);

#endif
