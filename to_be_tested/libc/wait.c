
extern int waitpid(int pid, int *status, int options);

int wait(int *status){
  return waitpid(-1 ,status, 0); 
}
