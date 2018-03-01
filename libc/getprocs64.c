/* implementation of our custom-made syscall getprocs64 */

#include <sys/defs.h>
#include <unistd.h>

static inline long syscall2(long syscall, long arg1, long arg2){
  long ret;
  __asm__ __volatile__("syscall" : "=a"(ret): "a"(syscall), "D"(arg1), "S"(arg2): "rcx", "r11", "memory");
  return ret;
}

size_t getprocs64(void *buf, size_t count){
  return syscall2((long)9, (long)buf, (long)count);   // random number for syscall now
}


