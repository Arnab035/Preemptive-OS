#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>

#define STDOUT_FILENO  1

typedef	unsigned short	u_short;
typedef	u_short	mode_t;

typedef struct rusage rusage;

struct process_info
{ 
  char pid[6];       // pid
  char ppid[6];     // parent pid
  char command[20];  // command
};

size_t getprocs64(void *buf, size_t count);

int open(const char *pathname, int flags, mode_t mode);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int unlink(const char *pathname);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);
int dup2(int filedes, int filedes2);
pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
int execve(const char *file, char *const argv[], char *const envp[]);
pid_t wait(int *status);
pid_t wait4(pid_t pid, int *wstatus, int options, rusage *rusage);
int waitpid(int pid, int *status, int options);

unsigned int sleep(unsigned int seconds);

pid_t getpid(void);
pid_t getppid(void);

// OPTIONAL: implement for ``on-disk r/w file system (+10 pts)''
off_t lseek(int fd, off_t offset, int whence);
int mkdir(const char *pathname, mode_t mode);

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);

#endif
