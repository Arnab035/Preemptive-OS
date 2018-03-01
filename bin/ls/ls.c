#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>

/* I changed the design to mimic bin/ps */

int main(int argc, char *argv[], char *envp[])
{
  DIR *dp;
  struct dirent *dir;
  int n_written = 0;  
  size_t max_buf_size = 1024;
  size_t point = 0;
  char buf[max_buf_size];
  if(argv[1] == NULL)
  {
    dp = opendir("/");
  }
  else
  {
    dp = opendir(argv[1]);
  }
  if(dp == NULL) exit(EXIT_FAILURE);
  
  n_written = readdir(dp, buf, max_buf_size);
  while(1)
  {
    if(point >= n_written)
      break;

    else
    {
      dir = (struct dirent *)(buf + point);
      printf("%s\n", dir->d_name);
      point += 32;  // sizeof(struct dirent)
    }
  }  
  int s = closedir(dp);
  if(s == -1) exit(EXIT_FAILURE);
  exit(EXIT_SUCCESS);
}


