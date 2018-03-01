
static inline long syscall1(long syscall, long arg1){
  long ret;
  __asm__ __volatile__("syscall" : "=a"(ret) : "a"(syscall), "D"(arg1) : "rcx","r11","memory");
  return ret;
}

int exit(int status){
  return syscall1((long)60, (long)status); 
}



