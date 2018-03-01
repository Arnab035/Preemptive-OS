#include <dirent.h>
#include <sys/defs.h>

static inline long syscall3(long syscall, long arg1, long arg2, long arg3)
{
  long ret;
  __asm__ __volatile__("syscall" : "=a"(ret): "a"(syscall),"D"(arg1),"S"(arg2),"d"(arg3) : "rcx","r11","memory");
  return ret;
}


int getdents(int fd, char *data, int count)
{
  return syscall3((long)8, (long)fd, (long)data, (long)count);
}

size_t readdir(DIR *dir, char *buf, int max_size)
{
  int fd = dir->fd;
  return getdents(fd, buf, max_size);  // returns the amount of data written
}
