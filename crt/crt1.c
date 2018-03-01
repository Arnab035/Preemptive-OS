#include <stdlib.h>

void _start(void) {
  // call main() and exit() here
  __asm__ __volatile__(
    "xorq %%rbp, %%rbp\n\t"
    "popq %%rdi\n\t"       // top of stack is argc
    "movq %%rsp, %%rsi\n\t"   
    "leaq 8(%%rsi, %%rdi, 8), %%rdx\n\t"   // envp = argv + 8 * argc + 8
    "pushq %%rbp\n\t"   // rsp + 8 aligns to 16 from AMD ABI
    "pushq %%rbp\n\t"
    "andq $0xfffffffffffffff0, %%rsp\n\t"
    "callq main\n\t"
    "movq %%rax, %%rdi\n\t"
    "callq exit\n\t"
    : : :
  );
}
