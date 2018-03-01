/* filesystem independent implementation of open and close */

#include <sys/fs.h>

int sys_open(const char *pathname, int flags, mode_t mode)
{
  // copy pathname from user to kernel
  if(pathname != NULL)
  {
    char *k_pathname = (char *)phys_mem_alloc();
    k_pathname = (char *)((uint64_t)k_pathname | KERNBASE);

    my_memcpy((void *)k_pathname, (void *)pathname, my_strlen(pathname));

    return tarfs_open(k_pathname, flags, mode);
  }
  else
  {
    return terminal_open(flags, mode);
  }
}

int tarfs_open(char *pathname, int flags, mode_t mode)
{
  struct pcb *current_process = get_current_process();
  char *tempname;
  /* pathname is from the userspace - it needs to be copied to kernel space */
  if(my_memcmp(pathname, ".", sizeof(char)) == 0)
  {
    my_memcpy((void *)pathname, "/bin/", 5);   // current working directory
  }
  else
  {
    /* no use of cd so -- let us keep it simple */
    tempname = (char *)phys_mem_alloc();
    tempname = (char *)((uint64_t)tempname | KERNBASE);
    my_memset((void *)tempname, 0, PAGE_BLOCK_SIZE);
    if(tempname[0] != '/')
    {
      my_memcpy((void *)tempname, "/", 1);
    }
    my_memcpy((void *)(tempname + 1), pathname, my_strlen(pathname));

    my_memcpy((void *)pathname, (void *)tempname, my_strlen(tempname));
  }
  
  int fd = available_fd(current_process);
  if(fd < 0)
    goto out;   /* release memory */
  
  struct dentry *d_entry = do_find_dentry(pathname, flags);  // TODO: investigate
  if(d_entry == NULL)
    goto out;     /* dentry not found - file non existent */

  struct file *fp = (struct file *)phys_mem_alloc();

  fp = (struct file *)((uint64_t)fp | KERNBASE);
  my_memset((void *)fp, 0, PAGE_BLOCK_SIZE);
  
  fp->f_offset = 0;
  fp->f_refcount += 1;

  fp->f_mode = mode;
  fp->f_flags = flags;
  fp->f_dentry = d_entry;
  
  /* the implementation of functions would of course depend on the filesystem you use
     readdir may be included once I have an idea */

  // TODO: BUG --> allocate memory
  fp->f_op = (struct file_operations *)phys_mem_alloc();
  fp->f_op = (struct file_operations *)((uint64_t)fp->f_op | KERNBASE);

  fp->f_op->open = NULL;
  fp->f_op->mmap = NULL;
  fp->f_op->read = tarfs_read;
  fp->f_op->write = NULL;
  fp->f_op->lseek = NULL;
  fp->f_op->close = tarfs_close;

  current_process->fd[fd] = fp;
  return fd;

  out:
    phys_mem_free((uint64_t)pathname);
    return -1;
}

int tarfs_close(struct file *fp)
{
  if(!fp) return -1;

  fp->f_refcount = fp->f_refcount - 1;

  if(fp->f_refcount <= 0)
  {
    /* deallocate the file entry */
    phys_mem_free((uint64_t)fp);
  }
  return 0;
}

ssize_t sys_read(int fd, void *buf, size_t count)
{
  struct pcb *curr_proc = get_current_process();
  struct file *fp = curr_proc->fd[fd];

  if(fp == NULL) return -1;
  
  return vfs_read(fp, buf, count, fp->f_offset);
}

ssize_t sys_write(int fd, void *buf, size_t count)
{
  struct pcb *curr_proc = get_current_process();
  struct file *fp = curr_proc->fd[fd];

  if(fp == NULL) return -1;

  return vfs_write(fp, buf, count, fp->f_offset);
}

int sys_close(int fd)
{
  struct pcb *curr_proc = get_current_process();
  struct file *fp = curr_proc->fd[fd];

  if(fp == NULL) return -1;

  return vfs_close(fp);
}

int available_fd(struct pcb *curr)
{
  int i = 0;
  struct pcb *curr_proc = curr;
  while(i < FD_MAX && curr_proc->fd[i] != NULL)
  {
    i++;
  }
  if(i == FD_MAX)
  {
    kprintf("exhausted all file descriptors\n");
    return -1; /* exhausted all file descriptors */
  }
  return i;
}

struct file *do_open_exec(const char *filename, int flags, int mode)
{
  char *k_pathname;
  k_pathname = (char *)phys_mem_alloc();
  k_pathname = (char *)((uint64_t)k_pathname | KERNBASE);
  /* copy filename array from user to kernel */
  my_memcpy((void *)k_pathname, (void *)filename, my_strlen(filename));

  struct dentry *d_entry = do_find_dentry(k_pathname, flags);
  if(d_entry == NULL)  /* non-existing file */
    goto out;

  /* exec also opens a file implicitly */
  struct pcb *current_process = get_current_process();

  int fd = available_fd(current_process);
  if(fd < 0)
    goto out;

  struct file *fp = (struct file *)phys_mem_alloc();
  fp = (struct file *)((uint64_t)fp | KERNBASE);

  my_memset((void *)fp, 0, PAGE_BLOCK_SIZE);
  fp->f_offset = 0;
  fp->f_refcount += 1;

  fp->f_mode = mode;
  fp->f_flags = flags;
  fp->f_dentry = d_entry;

  fp->f_op = (struct file_operations *)phys_mem_alloc();
  fp->f_op = (struct file_operations *)((uint64_t)fp->f_op | KERNBASE);

  fp->f_op->open = NULL;
  fp->f_op->mmap = NULL;    // generic mmap
  fp->f_op->read = tarfs_read;   // you are working with tarfs files only now

  fp->f_op->write = NULL;
  fp->f_op->lseek = NULL;
  fp->f_op->close = tarfs_close;

  current_process->fd[fd] = fp;
  return fp;

  out:
   //uint64_t phys_addr = get_kpa((uint64_t)k_pathname);
    phys_mem_free((uint64_t)k_pathname);
    return NULL;
}

