#include <unistd.h>

int waitpid(int pid, int *status, int options){
  return wait4(pid, status, 0, NULL); 
}
