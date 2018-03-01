
static inline long syscall3(long syscall, long arg1, long arg2, long arg3){
   long ret;
   __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3): "rcx","r11","memory");
   return ret;
}

int execve(const char* filename, char* const argv[], char* const envp[]){
   return syscall3((long)3, (long)filename, (long)argv, (long)envp);  // execve syscall number 3 in BLEH OS
}



