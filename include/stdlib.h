#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>

#define EXIT_FAILURE    1       /* Failing exit status.  */
#define EXIT_SUCCESS    0       /* Successful exit status.  */


int main(int argc, char *argv[], char *envp[]);
void exit(int status);
int setenv(const char *name, const char *value, int overwrite);
char *getenv(const char *name);
void *malloc(size_t size);
int my_munmap(void *ptr, size_t size);
int atoi(char *nptr);
int isdigit(int );

#endif
