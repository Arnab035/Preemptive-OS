#include "sys/vfs.h"

ssize_t vfs_read(struct file *file, void *buf, size_t size, off_t offset)
{
  if(file->f_op->read)  // get a callback
  {
    return file->f_op->read(file, (char *)buf, size, offset); 
  }
  return 0;   // 0 bytes read
}

ssize_t vfs_write(struct file *file, void *buf, size_t size, off_t offset)
{
  if(file->f_op->write)
  {
    return file->f_op->write(file, (char *)buf, size, offset);
  }
  return 0;    // 0 bytes written
}

/* dangerous to use */
uint32_t vfs_open(struct inode *file, uint64_t flags)
{
  /*
  if(file->f_op->open)
  {
    return file->f_op->open(file, flags);  
  }
  */
  return -1;  // open is usually not implemented by vfs - so if you use this func, please check
}

uint32_t vfs_close(struct file *fp)
{
  if(fp->f_op->close)
  {
    return fp->f_op->close(fp);
  }
  return -1; // close is also not implemented by vfs
}

/*
struct dirent *vfs_readdir(struct file *fp)
{
   should be a directory 
  if((fp->f_flags & O_DIRECTORY) && fp->readdir)
  {
    return (struct dirent *)file->readdir(file);
  }
  return NULL;
}
*/  
