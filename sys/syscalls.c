#include "sys/syscalls.h"
#include "sys/process.h"

//uint16_t syscall_max = sizeof(sys_call_table)/((uint64_t)sys_call_table[0]);

void *sys_call_table[] = {
                           &sys_read,
                           &sys_write,
                           &sys_fork,
                           &sys_execve,
                           &sys_wait,
                           &sys_exit,
                           &sys_open,
                           &sys_close,
                           &sys_getdents,
                           &sys_getprocs64,
                           &sys_mmap,
                           &sys_sleep,
                           &sys_kill,
                           &sys_munmap
                          };

/* Important args for syscalls 
   rax  system call number
 * rcx  return address
 * r11  saved rflags (note: r11 is callee-clobbered register in C ABI)
 * rdi  arg0
 * rsi  arg1
 * rdx  arg2
 * r10  arg3 (needs to be moved to rcx to conform to C ABI)
 * r8   arg4
 * r9   arg5
*/

void syscall_handler()
{
  do_syscalls();  // push caller saved registers inside this   
}

// user : code segment base, kcs : kernel code segment base

void init_syscall()
{
  //uint64_t efer_val;
  global_ds = (struct kernel_ds *)phys_mem_alloc();
  global_ds = (struct kernel_ds *)((uint64_t)global_ds | KERNBASE);

  uint64_t full_data = ((uint64_t)0x1B << 48) | ((uint64_t)0x8 << 32);
  uint32_t hi_data = full_data >> 32;   // high 32-bits
  uint32_t lo_data = full_data;   // low bits  

  do_wrmsr(MSR_STAR, lo_data, hi_data);  // MSR_STAR
  uint64_t addr = (uint64_t)do_syscalls;
 
  uint32_t hi_addr = addr >> 32;
  uint32_t lo_addr = addr;
  
  do_wrmsr(MSR_LSTAR, lo_addr, hi_addr);                     // MSR_LSTAR
                                                             // IA32_EFER
  uint32_t hi_efer, low_efer;
  do_rdmsr(IA32_EFER, &low_efer, &hi_efer);  

  low_efer |= 0x1;

  do_wrmsr(IA32_EFER, low_efer, hi_efer);    // EFER.SCE enable (IA32_EFER.SCE is 1st bit)
  
  uint64_t gs_kernel_base = (uint64_t)global_ds;
  uint32_t lo_gs_addr = gs_kernel_base;
  uint32_t hi_gs_addr = gs_kernel_base>>32;

  do_wrmsr(MSR_GS_KERNEL_BASE, lo_gs_addr, hi_gs_addr);  // for SWAPGS
}
