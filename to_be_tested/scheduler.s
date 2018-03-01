/*** moderatley equivalent to task.s where I switched stack space for tasks - now it is for processes ***/
/*** this assembly routine being called from the timer interrupt ***/

.section .text
.global  do_scheduler
.global  schedule
.global  current      //current is the current process executing
.global  set_tss_rsp  

do_scheduler:
  cli
  pushq %rax
  pushq %rbx
  pushq %rcx
  pushq %rdx
  pushq %rbp
  pushq %rdi
  pushq %rsi
  pushq %r8
  pushq %r9
  pushq %r10
  pushq %r11
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15
  movq (current), %rax
  cmpq %rax, $0
  jz   return_from_interrupt   // no process running
  push  %ds
  push  %es
  push  %fs
  push  %gs
  mov   $0x10, %ax    // kernel segments
  mov   %ax, %ds
  mov   %ax, %es
  mov   %ax, %fs
  mov   %ax, %gs
  movq  %rsp, (current)  // save the current stack pointer
  callq schedule
  movq  (current), %rsp  // restore the stack pointer
  movq  %rsp, %rdi
  callq set_tss_rsp      // call set_tss_rsp and save the kernel stack rsp
  pop  %gs
  pop  %fs
  pop  %es
  pop  %ds
 return_from_interrupt:
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %r11
  popq %r10
  popq %r9
  popq %r8
  popq %rsi
  popq %rdi
  popq %rbp
  popq %rdx
  popq %rcx
  popq %rbx
  popq %rax
  iretq
