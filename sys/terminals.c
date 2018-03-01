/* terminal driver control block and associated functions defined here */

/* initialize terminal -- TODO: maybe create a better defn of terminal and handle echo and raw/cooked mode -- this terminal by default always is cooked - you cannot make it raw */

#include <sys/terminals.h>

void ttyinit()
{
  input_buffer = (char *)phys_mem_alloc();
  input_buffer = (char *)((uint64_t)input_buffer | KERNBASE);

  clear_input_buffer();
}

int is_input_buffer_empty()
{
  return (line_cursor < PAGE_BLOCK_SIZE);  
}

int terminal_open(int flags, mode_t mode)
{
  /* no use for "/dev/tty" leave pathname as parameter */
  struct file *fp = (struct file *)phys_mem_alloc();
  fp = (struct file *)((uint64_t)fp | KERNBASE);
  
  int fd;

  my_memset((void *)fp, 0, PAGE_BLOCK_SIZE);
  fp->f_offset = 0;

  fp->f_refcount += 1;
  fp->f_mode = mode;
  fp->f_flags = flags;
  fp->f_dentry = NULL;

  fp->f_op = (struct file_operations *)phys_mem_alloc();
  fp->f_op = (struct file_operations *)((uint64_t)fp->f_op | KERNBASE);

  fp->f_op->open = NULL;
  fp->f_op->mmap = NULL;
  fp->f_op->write = terminal_write;
  fp->f_op->read = terminal_read;
  fp->f_op->lseek = NULL;    /* not needed for terminals */
  fp->f_op->close = terminal_close;

  struct pcb *current_process = get_current_process();
  /* open stdin, stdout, stderr */
  fd = available_fd(current_process);  /* should return 0/1/2 */
  if(fd < 0)
    goto out;

  current_process->fd[fd] = fp;
  return fd;

 out:
  phys_mem_free((uint64_t)fp);
  return -1;
}

ssize_t terminal_read(struct file *fp, char *buf, size_t size, off_t offset)
{
  if(!fp)
  {
    kprintf("Invalid terminal file descriptor\n");
    return -1;
  }
  size_t temp_size;
  int i;
  /* read from the input buffer - check for a /n  character - if no there- doing a busy wait now- not blocking */
  while(1)
  {
    if(check_input_buffer())
    {
      break;
    }
  }
  //kprintf("line cursor is %d\n", line_cursor);
  /* after task is waken up, it starts from here */
  temp_size = (size < line_cursor) ? size : line_cursor;
  i = 0;
  //kprintf("temp_size is %d\n", temp_size);
  while(temp_size-- > 0) // memcpy ? //
  {
    buf[i] = input_buffer[i];
    i++;
  }
  buf[i] = '\0';
  clear_input_buffer();
  return i;
}

/* terminal write directly writes to the screen / video memory (no use of output buffers here */

ssize_t terminal_write(struct file *fp, char *buf, size_t size, off_t offset)
{
  if(!fp)
  {
    kprintf("Invalid terminal file descriptor\n"); 
    return -1;
  }
  int i;
  char c;
  for(i = 0; i < size; i++)
  {
    c = buf[i];
    kputchar(c);
  }
  return i;  
}

/* should be called when the init_task is being killed */
int terminal_close(struct file *fp)
{
  if(!fp) return -1;

  fp->f_refcount = fp->f_refcount - 1;
  if(fp->f_refcount <= 0)
  {
    phys_mem_free((uint64_t)fp);
  }
  return 0;
}

/* check input buffer tells you if you need to block or not */
/* line cursor tells you how many characters are there in the input buffer */

int check_input_buffer()
{
  int i = 0;
  //kprintf("line cursor is %d\n", line_cursor);
  for(; i < line_cursor; i++)
  {
    if(input_buffer[i] == '\n')
      return 1;    
  }
  return 0;
}

void insert_char_into_input_buffer(char c)
{
  *head = c;
   /* add some checks here for newline to wake process up -- backspace is handled */
   head++;
   line_cursor++;
}

void erase_char_from_input_buffer()
{
  head--;
  line_cursor--;
}

void clear_input_buffer()
{
  head = input_buffer;
  line_cursor = 0;
}

