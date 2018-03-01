// fgetc

#include <stdio.h>
#include <sys/defs.h>
#include <unistd.h>
#include <stdlib.h>

int fgetc(FILE *fp){
  char c;
  if ( read(fp->fd, &c, 1) == 0) return (EOF);
  putchar(c);
  return (unsigned char)c;
}