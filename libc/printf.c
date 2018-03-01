#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

/* Writes the C string pointed by format to the standard output (stdout). If format includes format specifiers (subsequences beginning with %), the additional arguments following format are formatted and inserted in the resulting string replacing their respective specifiers.
*/

size_t vprintf(char *, size_t, const char *, va_list);

int printf(const char *format, ...)
{
  va_list arg;
  char buf[128];  // max length is 4096 for the buffer //
  size_t n_written;
  //puts("print starts");
  if(format == NULL)
  {
    return 0;  // number of chars printed is 0
  }
  va_start(arg, format);
  n_written = vprintf((void *)buf, 128, format, arg);  // cannot pass format directly - this is a wrapper 
  va_end(arg);  

  return write(1, (void *)buf, n_written/* count to be returned by the next function called vprintf */);    
}
