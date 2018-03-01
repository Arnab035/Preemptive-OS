// ***sleep implementation*** //

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp)
{
  int seconds;
  int err;

  if(argc <= 1)
    exit(1);

  seconds = atoi(argv[1]);

  err = sleep(seconds);  // different syscall ?
  //printf("task waking up from sleep with error is %d\n", err);
  return (err < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
