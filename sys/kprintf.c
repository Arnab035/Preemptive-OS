#include <sys/kprintf.h>
#include <stdarg.h>
#include <sys/defs.h>
#include <sys/mmgr.h>
#include <sys/util.h>

// black and white color now

unsigned int x = 0;
unsigned int y = 0;
unsigned short *vgamemory;
int color = 0x0A;

void kputchar(const char c)
{
  // actual kernel printf character function
  // anything greater than space will be printed -- not performing scrolling
  unsigned short *address;
  x = get_x();
  y = get_y();
  if(is_paging == 0) vgamemory = (unsigned short *)0xB8000;
  else vgamemory = (unsigned short *)(0xFFFFFFFF80000000 + 0xB8000);
  
  if(c >= ' '){
    address = vgamemory + ((y * 80) + x);
    *address = c | (color << 8);
    x++;
  }
  else if(c == '\r'){
    x = 0;
  }
  else if(c == '\n'){
    x = 0;
    y += 1;
  }
  if(x >= 80){
    x = 0;
    y += 1;
  }    
}

void kputs(const char *s){
  for(; *s != '\0'; s++)
    kputchar(*s);
}

char *itoa(unsigned int i)
{
  const char *available = "0123456789";
  char p[6];  // pid is a max of 65536
  char *s = (char *)phys_mem_alloc();
  s = (char *)((uint64_t)s | KERNBASE);
  unsigned int ret = i;
  int j = 5;
  int index;
  p[j] = '\0';
  int k;
  if(ret == 0)
  {
    p[--j] = available[0];
  }
  while(ret != 0)
  {
    p[--j] = available[ret % 10];
    ret = ret/10;
  }
  for(k=j, index=0; p[k] != '\0'; k++, index++)
  {
    s[index] = p[k];
  }
  s[index] = '\0';
  return s; 
}

void convert_to_base(unsigned int i, int base, char *s)
{
  const char *available = "0123456789ABCDEF";
  // expect no. of chars to be max 64 -- if go beyond 64 invalid memory
  char p[128];
  unsigned int ret = i;
  int j = 127;
  p[j] = '\0';
  int k;
  if(ret == 0){
     p[--j] = available[0];
  }
  while(ret != 0){
    // go reverse
    p[--j] = available[ret % base];
    ret = ret / base;
  }
  for(k=j; p[k] != '\0'; k++) *s++ = p[k];
  *s = '\0'; 
}

// functions to get and set the x and y coordinates for printing the characters to the screen

void set_x(unsigned int _x){
  x = _x;
}

void set_y(unsigned int _y){
  y = _y;
}

unsigned int get_x(){
  return x;
}

unsigned int get_y(){
  return y;
}

/*
int my_strlen(const char *s){
  if(s == NULL) return 0;
  int len = 0;
  for(; *s != '\0'; s++) len++;
  return len;
}*/

void my_strcpy(char *s, const char *t){
  for(; *t != '\0'; t++) *s++ = *t;
  *s = '\0';
}


void kprintf(const char *fmt, ...)
{
  int i;
  va_list arg;
  const char *str = fmt;
  va_start(arg, fmt);
  char ret[64];
  const char *s;
  int j;
  for(j=0; j < my_strlen(str); j++){
    if(str[j] != '%') kputchar(str[j]);
    else{
      j++;
      // max 64
      switch(str[j])
      {
       case 'd':
    	i = va_arg(arg, int);
        if(i < 0){
	  i = -i;
          kputchar('-');
          convert_to_base(i, 10, ret);
          kputs(ret);
        }
        else{
          convert_to_base(i,10,ret);
          kputs(ret);
        }
        break;

       case 'c':
        i = va_arg(arg, int);
        kputchar(i);
        break;
       case 'x':  // hexadecimal
	i = va_arg(arg, int);
        convert_to_base(i, 16, ret);
        kputs(ret);
        break;
       case 'p':  // pointer
	i = va_arg(arg, int);
        convert_to_base(i, 16, ret);
        kputs(ret);
        break; 
       case 's':  // string, 
	s = va_arg(arg, const char *);
        kputs(s);
        break;
       case 'o':  // octal
	i = va_arg(arg, int);
        convert_to_base(i, 8, ret);
        kputs(ret);
        break;
      }
    }
   }
  va_end(arg);
}
