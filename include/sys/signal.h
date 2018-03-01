/* the signal.h ---> implementation of all kill and signals */

#include <sys/defs.h>
#include <sys/process.h>
#include <sys/scheduler.h>
#include <sys/kprintf.h>

#define SIGCHLD 17

int sys_kill(pid_t, int);

int send_signal(pid_t, int);
