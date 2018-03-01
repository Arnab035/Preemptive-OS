/* this file only contains utility function definitions */

#include <sys/util.h>

uint64_t read_cr3()
{
  uint64_t ret;
  __asm__ __volatile__("movq %%cr3, %0\n\t": "=r"(ret): : );
  return ret;
}

uint64_t read_cr2()
{
  uint64_t ret;
  __asm__ __volatile__("movq %%cr2, %0\n\t": "=r"(ret): : );
  return ret;
}

/* from Anon piazza post - syscall MSR_LSTAR not working */
void do_wrmsr(uint32_t msr_reg, uint32_t lo_data, uint32_t hi_data)
{
  __asm__ __volatile__(
                        "wrmsr\n\t": :"c"(msr_reg), "a"(lo_data), "d"(hi_data):
                      );
}

void do_rdmsr(uint32_t msr_reg, uint32_t *lo_data, uint32_t *hi_data)
{
  __asm__ __volatile__(
                        "rdmsr\n\t": "=a"(*lo_data), "=d"(*hi_data) :"c"(msr_reg) :
                      );
}

uint64_t page_round_up(uint64_t addr)
{
  if(addr < 4096) { return addr; }; 
  return addr & ~(0xFFF);
}

uint64_t page_round_down(uint64_t addr)
{
  addr = (addr + 0xFFF);
  addr = addr & ~(0xFFF);
  return addr;
}

uint64_t oct2dec(unsigned char *data, uint8_t size)
{
  uint64_t n = 0;
  unsigned char *c = data;
  while(size-- > 0)
  {
    n *= 8;
    n += (*c - '0');
    c++;
  }
  return n;
}

int my_memcmp(void *s1, void *s2, int n)
{
  unsigned char *t1 = (unsigned char *)s1;
  unsigned char *t2 = (unsigned char *)s2;

  int diff = 0;
  while(n-- > 0)
  {
    diff += (*t1-'0') - (*t2-'0');
    t1++;
    t2++;
  }
  return diff;
}

void my_memset(void *s, int c, size_t n)
{
  if(n==0 || s== NULL) return;
  unsigned char *p = s;
  int i = 0;
  for(; i < n; i++) *p++ = c;
  *p = '\0';
}

int my_strlen(const char *s)
{
  int len = 0;
  int i = 0;
  while(*(s + i) != '\0')
  {
    len += 1;
    i += 1;
  }
  return len;
}

void my_memcpy(void *dest, void *src, int n)
{
  char *dest1 = (char *)dest;
  char *src1 = (char *)src;
  int i;
  for(i = 0; i < n; i++)
    dest1[i] = src1[i]; 
}

char *my_strtok(char s[], char *ct)
{
  static char *nextS = NULL;
  static int done = 1;

  if(s != NULL)
  {
    if(*s == '\0'){done = 1; return NULL;}
    nextS = s;
    while((my_strchr(ct, *nextS) == NULL) && (*nextS != '\0')) nextS++;
    done = (*nextS == '\0');
    *nextS = '\0';
    return s;
  }
  if(done) return NULL;
  s = ++nextS;
  while((my_strchr(ct, *nextS) == NULL) && (*nextS != '\0')) nextS++;
  done = (*nextS == '\0');
  *nextS = '\0';
  return s;
}

/*
Appends a copy of the source string to the destination string. The terminating null character in destination is overwritten by the first character of source, and a null-character is included at the end of the new string formed by the concatenation of both in destination.
*/

char *my_strcat(char *dest, char *src)
{
  char *newS = (char *)phys_mem_alloc();
  newS = (char *)((uint64_t)newS | KERNBASE);  

  int i = 0;
  int j;
  while(dest[i] != '\0') { newS[i] = dest[i]; i++; }
  j = 0;
  while(src[j] != '\0') { newS[i] = src[j]; i++; j++; }
  newS[i] = '\0';
  return newS;
}

uint16_t get_pid()
{
  last_pid = last_pid + 1;
  return last_pid;
}

uint64_t page_mask(uint64_t addr)
{
  uint64_t masked_addr;
  masked_addr = addr & ~(PAGE_BLOCK_SIZE - 1);
  return masked_addr;
}

char *my_strchr(char *cs, int c)
{
  if(!cs) return NULL;
  int i;
  char *temp;
  for(i = 0; *(cs + i) != '\0'; i++)
  {
    if(*(cs + i) == c)
    {
      temp = cs + i;
      return temp;
    }
  }
  return NULL;
}
