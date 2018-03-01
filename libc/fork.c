#include <unistd.h>
// fork requires no args to pass only the syscall number

static inline long syscall0(long syscall){
  long ret;
  __asm__ __volatile__ ("syscall" : "=a" (ret) : "a"(syscall) : "rcx","r11","memory");
  return ret;
}

pid_t fork(){
  return syscall0((long)2);  // fork system call number 2 here  		
}


