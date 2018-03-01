/**** sleep is implemented directly as a system call ****/

// TODO: add the system call handler //

static inline long syscall1(long syscall, unsigned long arg1)
{
  long ret;
  __asm__ __volatile__("syscall" : "=a"(ret): "a"(syscall), "D"(arg1));
  return ret;
}


int sleep(unsigned long seconds)
{
  return syscall1(11, seconds);   // add syscall number
}
