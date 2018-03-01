/* should contain ps binary implementation */

/* this requires a hack- ps usually opens the proc files to read all the active and zombie processes , for 506 we cannot use these files, so do a system call that directly tries to read from the allproc and zombproc process list */

/*struct process_info : process table information (this can also contain cmds - but now I have no time :< )  */

// TODO: implement printf

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv, char **envp)
{
  struct process_info *procfs;
  
  size_t n_written;
  size_t max_buf_size = 1024; /* big enough */
  size_t point = 0;
  char buf[max_buf_size];
  
  n_written = getprocs64(buf, max_buf_size);
  while(1)
  {
    if(point >= n_written)
      break;
    
    else
    {
      procfs = (struct process_info *)(buf + point);
      printf(" Process PID : %s Parent PID : %s Cmd: %s\n", procfs->pid, procfs->ppid, procfs->command);    
      point += 32;
    }
  }
  exit(EXIT_SUCCESS);
}



