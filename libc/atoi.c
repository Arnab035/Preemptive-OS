/* atoi implementation */

#include <stdlib.h>

int atoi(char *nptr)
{
  int value = 0;
  //printf("nptr is %s\n", nptr);
  if(!nptr) return value;

  int i = 0;
  for(; nptr[i] != '\0'; ++i)
  {
    value = value * 10 + (nptr[i] - '0');
  }
  return value;
}
