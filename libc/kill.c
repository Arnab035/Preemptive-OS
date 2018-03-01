#include <unistd.h>
#include <sys/defs.h>

static inline long syscall2(long syscall, long arg1, long arg2){
  long ret;
  __asm__ __volatile__("syscall" : "=a"(ret) : "a"(syscall), "D"(arg1), "S"(arg2): "rcx", "r11", "memory");
  return ret;
}

int kill(pid_t pid, int sig){
  return syscall2((long)12, (long)pid, (long)sig);
}
