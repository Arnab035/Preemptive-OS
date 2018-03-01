
#include <stdio.h>
#include <unistd.h>


int getc(FILE *fp){
  static char buf[1024];
  static char *bufp = buf;
  static int n = 0;

  if (n == 0) /* buffer is empty */
    {
        n = read(fp->fd, buf, sizeof buf);
        bufp = buf;
    }

    return (--n >= 0) ? (unsigned char) *bufp++ : EOF;
}
