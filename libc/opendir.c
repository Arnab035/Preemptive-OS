//#include <stddef.h> -- add size_t definitions

#include <dirent.h> 
#include <stdlib.h>
#include <stdio.h>

#define O_RDONLY        00000000
#define O_DIRECTORY     00000002

int close(int fp);

int open(const char *pathname, int flags, int mode);

DIR *alloc_dir(int fd)
{
  DIR *dirp = (DIR *) malloc (sizeof (DIR));
  //printf("dirp is 0x%x\n", dirp);
  
  if(dirp == NULL)
  {
    close(fd);
    return NULL;
  }
  //printf("dirp->fd is %d\n", dirp->fd);
  dirp->fd = fd;
  return dirp;
}

DIR *opendir(const char *dirname)
{
  if(dirname == NULL) return NULL;
  int fd = open(dirname, O_RDONLY | O_DIRECTORY, 0);
  if(fd == -1) return NULL;
  return alloc_dir(fd);
}


