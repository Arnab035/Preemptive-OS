
#include <unistd.h>
#include <stdio.h>

int putchar(int c)
{
  // write character to stdout -- use write() system call
  // write(int fd, void* buffer, int count);
  char x = c;
  int ret;
  ret = write(1, &x, 1);
  if(ret == 1)
  	return c;
  return EOF;
}

