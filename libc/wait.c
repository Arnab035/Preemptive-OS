#include <unistd.h>

static inline long syscall1(long syscall, long arg1)
{
  long ret;
  __asm__ __volatile__(
                         "syscall"
                        :"=a"(ret) : "a"(syscall), "D"(arg1): "rcx", "r11", "memory");
                       
  return ret;
}

//extern int waitpid(int pid, int *status, int options);

pid_t wait(int *status){
  return syscall1((long)4 , (long)status);   // wait system call in BLEH OS is 4
}
