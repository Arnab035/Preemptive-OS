
#include <string.h>

char *strcpy(char *dst, char *src)
{
  char *temp = dst;
  while(*src != '\0')
  {
    *dst = *src;
    dst++;
    src++;
  }
  return temp;
}
