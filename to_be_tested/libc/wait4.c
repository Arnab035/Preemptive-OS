#include <unistd.h>

static inline long syscall4(long syscall, long arg1, long arg2, long arg3, long arg4){
  long ret;
  register long r10 __asm__("r10") = arg4;
  __asm__ __volatile__ (
                         "syscall" 
			: "=a" (ret) : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10): "rcx","r11", "memory");  
  return ret;
}

pid_t wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage){
  return syscall4((long)61,(long)pid, (long)wstatus, (long)options, (long)rusage);
}
