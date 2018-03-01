/* implementation of kill */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char **argv, char **envp)
{
  int signal;
  int pid, pids, err;
  if(argc <= 2)
  {
    puts("usage : kill -9 <pid>\n");
    return EXIT_FAILURE;
  }

  if(argv[1][0] == '-') /* kill -9 */
  {
    if(!isdigit(argv[1][1]))
    {
      puts("usage : kill -9 <pid>\n");
      return EXIT_FAILURE;
    }
    signal = argv[1][1] - '0';
    if(signal != 9)
    {
      puts("unknown signal number\n");
      return EXIT_FAILURE;
    }
  }
  pids = 2;
  for(; pids < argc; pids++)
  {
    pid = atoi(argv[pids]);
    err = kill(pid, signal);
    if(err < 0)
    {
      puts("error implementing kill\n");
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
