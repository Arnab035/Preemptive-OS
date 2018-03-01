
// open requires mode as well

static inline long syscall3(long syscall, const char* arg1, long arg2, long arg3){
  long ret;
  __asm__ __volatile__("syscall" : "=a"(ret): "a"(syscall),"D"(arg1),"S"(arg2),"d"(arg3) : "rcx","r11","memory");
  return ret;
}



int open(const char *pathname, int flags, int mode){
  
  return syscall3((long)6, pathname, (long)flags, (long)mode);
}
