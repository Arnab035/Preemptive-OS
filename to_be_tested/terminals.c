// useful terminal filesystem abstractions ---

/*
  has two functions that you need to work with
 
  -------  terminal_read  (similar in concept to tarfs_read)
  -------  terminal_write (similar in concept to tarfs_write)
*/

#include <sys/terminals.h>

ssize_t terminal_read(void *buf, ssize_t count)
{
  char *ptr = term_head;
  int cnt = 0;
  while(ptr <= term_tail)
  {
    if(cnt == count)
    {
      my_memcpy(buf, (void *)term_head, cnt);
      return;
    }
    else if(*ptr == '\n')
    {
      my_memcpy(buf, (void *)term_head, cnt);
      return;
    }
    ptr++;
    cnt++;
  }
  struct pcb *curr = get_current_process();
  curr->thread_state = SLEEPING;
  insert_into_readwaitqueue(curr);  // if newline not found or cnt == count, block process
}

ssize_t terminal_write(void *buf, ssize_t count)
{
  // same mechanism as kprintf
  /*
   write() writes up to count bytes from the buffer starting at buf to
       the kprintf function.
  */
  ssize_t i = 0;
  for(; i <= count; i++)
  {
    const char c = buf[i];
    kputchar(c);  // kputchar from the kprintf function
  }
}


