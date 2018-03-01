#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// int execvpe(const char *file, char *const argv[], char *const envp[]);
// execvpe calls execve internally
// The execlp(), execvp(), and execvpe() functions duplicate the actions of the shell in searching for 
// an executable file if the specified filename does not contain a slash (/) character. 
 // The file is sought in the colon-separated list of directory pathnames specified in the PATH environment variable. 
 // If this variable isn't defined, the path list defaults to the current directory followed by the list of directories returned by confstr(_CS_PATH).

#define FIX_PATH "/bin:/usr/bin"

extern int execve(const char* filename, char* const argv[], char* const envp[]);

extern char *getenv(const char *name);

int execvpe(const char *file, char *const argv[], char *const envp[]){
    char *pre;
    int len;
    if(strchr(file, '/'))
    	execve(file, argv, envp);
    char *searchpath = NULL /*= getenv("PATH")*/;
    if(searchpath == NULL){
    	searchpath = envp[0]/*FIX_PATH*/;
    }
    pre = strtok(searchpath, ":");   /* one env variable */
    len = strlen(pre) + 1 + strlen(file);
    char path[len];
    memcpy(path, pre, strlen(pre));
    path[strlen(pre)] = '/';
    memcpy(path+strlen(pre)+1, file, strlen(file));
    path[len] = '\0';
    puts(path);
    //puts("About to execve\n");
    execve(path, argv, envp);
    //pre = strtok(searchpath, ":");
    /*
    while(pre != NULL){
    	char *path; 
    	len = strlen(pre) + 1 + strlen(file);
    	path = malloc(len*sizeof(char));
    	memcpy(path, searchpath, strlen(pre));
    	path[strlen(pre)] = '/';
    	memcpy(path + strlen(pre) + 1, file, strlen(file));
        
    	execve(path, argv, envp);
    	//if(errno == E2BIG  || errno == ENOEXEC || errno == ENOMEM || errno == ETXTBSY) break;
    	pre = strtok(NULL,":");
    }
    */
    return -1;
 }


