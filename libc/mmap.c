
#include <sys/defs.h>

static inline long syscall6(long syscall, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6){
  long ret;
  register long r10 __asm__("r10") = arg4;
  register long r8 __asm__("r8") = arg5;
  register long r9 __asm__("r9") = arg6;


  __asm__ __volatile__ ("syscall"
  		         :"=a"(ret):"a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9):
  				 "memory","rcx","r11");
  return ret;
}

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, int off){
  return (void *)syscall6((long)10, (long)addr, (long)len, (long)prot, (long)flags, (long)fildes, (long)off);
}
