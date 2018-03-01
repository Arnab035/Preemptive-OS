/*** moderately equivalent to task.s where I switched stack space for tasks - now it is for processes ***/
/*** this assembly routine being called from the timer interrupt ***/

.global do_switch
.global post_stack_switch

.section .text

do_switch:
  pushq %rbp
  pushq %rbx
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15
  movq %rsp, 8(%rdi)   // add 8 to whatever is in rdi
  movq 8(%rsi), %rsp 
  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %rbx
  popq %rbp
  jmp  post_stack_switch  // set up the esp0 value

