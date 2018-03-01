/*********

  this is the page fault handler starter code

*********/

.section .text
.global  page_fault_handler
.global  page_fault_handler_wrapper

page_fault_handler_wrapper:
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
  cld
  movq 60(%rsp), %rdi   // error code
  movq %cr2, %rsi       // faulting address
  callq page_fault_handler
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
  addq %rsp, 8
  iretq
  
