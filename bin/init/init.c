/* finally reached init processes */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[])
{
  pid_t bleh_pid;
  char *const sbush_argv[] = {"bin/sbush", NULL};

  char *const sbush_envp[] = {"/bin:", NULL};

  bleh_pid = fork();
  //char *path = "rootfs/";
  //puts("fork returned\n");
  if(bleh_pid < 0)
  {
    puts("There is a bug in fork\n");
    exit(1);
  }
  else if(bleh_pid == 0)
  {
    /* inside child processes */
    //chdir(path);  // TODO: implement chdir
    //puts("execve");
    execve("/bin/sbush", sbush_argv, sbush_envp);

    puts("sbush has returned\n");
    exit(1);
  }
  /* wait for zombie processes*/
  else
  {
    while(1)
    {
      int status;
      pid_t waitpid;

      waitpid = wait(&status);
      //puts("parent process going for wait\n");
      if(waitpid == -1)
      {
        puts("error in wait\n");
        break;
      }
      else
      {
        printf("/bin/init: reaped pid : %d\n", waitpid);
        if(waitpid == 2)
        {
          printf("You are killing the shell.. \n");
        } 
      }
    }
  }
  return 0;
}
