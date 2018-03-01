
static inline long syscall2(long syscall, long arg1, long arg2){
  long ret;
  __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(syscall), "D"(arg1), "S"(arg2): "rcx","r11","memory");
  return ret;
}

int dup2(int filedes, int filedes2){
  return syscall2((long)33, (long)filedes, (long)filedes2);
}
