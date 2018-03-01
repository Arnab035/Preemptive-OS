
#include <sys/defs.h>

static inline long syscall3(long syscall, long arg1, long arg2, long arg3){
  long ret;
  __asm__ __volatile__ ("syscall" : "=a" (ret) : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3) : "rcx","r11", "memory");
  return ret;
}


ssize_t write(int fd, void *c, int count){
  return syscall3((long)1, (long)fd, (long)c, (long)count);
}


