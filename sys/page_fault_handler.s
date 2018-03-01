/*********

  this is the page fault handler starter code - requires modification 

*********/

.section .text
.global copy_regs
.global page_fault_handler
.global i86_page_fault_handler

i86_page_fault_handler:
  addq  $-(120), %rsp
  cld
  movq %r11, 6*8(%rsp)
  movq %r10, 7*8(%rsp)
  movq %r9,  8*8(%rsp)
  movq %r8,  9*8(%rsp)
  movq %rax, 10*8(%rsp)
  movq %rcx, 11*8(%rsp)
  movq %rdx, 12*8(%rsp)
  movq %rsi, 13*8(%rsp)
  movq %rdi, 14*8(%rsp)
  movq %rbx, 5*8(%rsp)
  movq %rbp, 4*8(%rsp)
  movq %r12, 3*8(%rsp)
  movq %r13, 2*8(%rsp)
  movq %r14, 1*8(%rsp)
  movq %r15, 0*8(%rsp)
  /* so we switch to kernel stack */
  movq %rsp, %rdi
  callq copy_regs
  movq %rax, %rsp
  movq %rsp, %rdi
  movq 15*8(%rsp), %rsi		/* get error code */
  movq $-1, 15*8(%rsp)		/* no syscall */
  callq page_fault_handler
  /* restore registers */
  cli
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %rbp
  popq %rbx
  popq %r11
  popq %r10
  popq %r9
  popq %r8
  popq %rax
  popq %rcx
  popq %rdx
  popq %rsi
  popq %rdi
  subq $(-8), %rsp
  iretq                       /* moves to userspace */
