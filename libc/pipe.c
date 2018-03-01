
static inline long syscall1(long syscall, long arg1){
  long ret;
  __asm__ __volatile__ ("syscall" : "=a" (ret): "a"(syscall), "D"(arg1): "rcx","r11","cc","memory");
  return ret;
}

int pipe(int filedes[]){
  return syscall1((long)22, (long)filedes);
}


