/** sys/wait.h implementation */

#ifndef __WAIT_H
#define __WAIT_H

/* useful macros */

#define WTERMSIG(s) ((s) & 0x7f)
#define WIFEXITED(s) (!WTERMSIG(s))
#define WIFSIGNALED(s) (s == 9 || s == 11)   /*sigsegv and sigkill */


#endif
