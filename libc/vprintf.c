#include <sys/defs.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void convert_to_base(unsigned int i, int base, char *s)
{
  const char *available = "0123456789ABCDEF";
  char p[64];
  unsigned int ret = i;
  int j = 63;
  p[j] = '\0';
  int k;
  if(ret == 0)
  {
    p[--j] = available[0];
  }
  while(ret != 0)
  {
    p[--j] = available[ret % base];
    ret = ret/base;
  }
  for(k=j;p[k]!='\0';k++) *s++=p[k];
  *s='\0';
}

size_t vprintf(char *buf, size_t size, const char *fmt, va_list arg)
{
  // store the whole string into buf, and return the number of chars stored //
  char *temp = buf;
  const char *str = fmt;
  if(str == NULL) return 0;
  if(*str == 0)
  {
    *buf = '\0';
    return 1;
  }
  //int args;
  const char *s;
  size_t n_written = 0;
  int len, j, i, args;
  //char *towrite;  
  for(j=0; j<strlen(str); j++)
  {
    char ret[64] = {0};
    
    if(str[j] != '%')
    {
      *temp = str[j];
      temp++; n_written++;
    }
    else
    {
      j++;
      switch(str[j])
      {
        case 's':
          s = va_arg(arg, const char *);
          //towrite = s;
          len = strlen(s);
          for(i=0;i<len;i++)
          {
            temp[i] = s[i];
          }
          temp += len;
          n_written += len;
          break;
        case 'd':
          args = va_arg(arg, int);
          if(args<0)
          {
            args = -args;
            convert_to_base(args, 10, ret);
            len = strlen(ret);
            for(i=0;i<len;i++)
            {
              temp[i] = ret[i];
            }
            
          }
          else
          {
            convert_to_base(args, 10, ret);
            len=strlen(ret);
            for(i=0;i<len;i++)
            {
              temp[i]=ret[i];
            }
          }
          temp+=len;
          n_written+=len;
          break;
        case 'c':
          args = va_arg(arg, int);
          temp[0] = args;
          temp++; n_written++;
          break;
        case 'x':
          args = va_arg(arg, int);
          convert_to_base(args, 16, ret);
          len=strlen(ret);
          for(i=0;i<len;i++)
          {
            temp[i]=ret[i];
          }
          temp+=len;
          n_written+=len;
          break;
        case 'p':
          args = va_arg(arg, int);
          convert_to_base(args, 16, ret);
          len=strlen(ret);
          for(i=0;i<len;i++)
          {
            temp[i]=ret[i];
          }
          temp+=len;
          n_written+=len;
          break;
        case 'o':
          args = va_arg(arg, int);
          convert_to_base(args, 8, ret);
          len=strlen(ret);
          for(i=0;i<len;i++)
          {
            temp[i]=ret[i];
          }
          temp+=len;
          n_written+=len;
          break;
      } 
    }
  }
  *temp = '\0';   // do not forget
  return n_written;
}
