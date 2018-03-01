.section .text
.global  do_syscalls
.global  sys_call_table
.global  ret_from_fork
.global  switch_to_user_cr3

do_syscalls:
  swapgs           // TODO: obtain the kernel stack address to be pushed into the MSR associated
  movq  %rsp, %gs:8   // save user rsp
  movq  %gs:0, %rsp   // load kernel rsp
  pushq $0x23       //user stack segment to be used by sysret
  pushq %gs:8       //user stack pointer
  sti               // enable interrupts
  pushq %r11        // saved rflags
  pushq $0x2B       // user code segment
  pushq %rcx        // return address
  pushq %rax        // system call number  registers->rax
  pushq %rdi        // arg0
  pushq %rsi        // arg1
  pushq %rdx        // arg2
  pushq %rcx
  pushq	$-1          // ENOSYS /* pt_regs->ax to be returned */
  pushq %r8
  pushq %r9
  pushq %r10
  pushq %r11
  subq  $(6*8), %rsp               // make way for the callee saved registers like rbx, r12, r13 which have been saved automatically etc.
  cmpq  $syscall_max, %rax
  ja    1f
  movq  %r10, %rcx
  callq  *sys_call_table(, %rax, 8)  // sys_call_table needs to be defined
1:
  cli                      // disable interrupts
  movq %rax, 80(%rsp)
  movq 128(%rsp), %rcx  // restore original value of return address
  movq 144(%rsp), %r11  // restore original value of rflags
  movq 56(%rsp), %r10
  movq 64(%rsp), %r9
  movq 72(%rsp), %r8
  movq 80(%rsp), %rax
  movq 96(%rsp), %rdx
  movq 104(%rsp), %rsi
  movq 112(%rsp), %rdi
  movq 152(%rsp), %rsp
  swapgs                // swapgs  to restore msr_kernel_base
  sysretq               // sysretq to return from system call

/* child process returns to execute this function */
ret_from_fork:
  andq %rbx, %rbx   // determine if is a kernel thread
  jnz  2f
3:
  //prepare to move to userspace
  callq switch_to_user_cr3
  movq (%rsp), %r15
  movq 8(%rsp), %r14
  movq 16(%rsp), %r13
  movq 24(%rsp), %r12
  movq 32(%rsp), %rbp
  movq 40(%rsp), %rbx
  movq 48(%rsp), %r11
  movq 56(%rsp), %r10
  movq 64(%rsp), %r9
  movq 72(%rsp), %r8
  movq 80(%rsp), %rax
  movq 88(%rsp), %rcx
  movq 96(%rsp), %rdx
  movq 104(%rsp), %rsi
  movq 112(%rsp), %rdi
  subq $-(128), %rsp
  iretq              /* rsp is expected to point to iret frame now */
2:
  movq %r12, %rdi
  callq *%rbx
  movq $0, 80(%rsp)
  jmp  3b   
