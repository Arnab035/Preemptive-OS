#ifndef _DIRENT_H
#define _DIRENT_H
#include <sys/defs.h>

#define NAME_MAX 31

typedef struct dirent {
 char d_name[NAME_MAX+1];  // max length of filename is 32 - buffer will overflow for larger names (for CSE506 you can afford)
} dirent;

typedef struct dir{
  int fd;
} DIR;

DIR *opendir(const char *name);
size_t readdir(DIR *dirp, char *buf, int size);
int closedir(DIR *dirp);

#endif
