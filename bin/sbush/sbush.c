#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h> 

char** environ;

char* collect_shell_command();
char* collect_shell_command_from_file(FILE *);
char** tokenize_string(char* );

int is_whitespace(unsigned char );

void execute(char** );

//void execute_builtin(char**);
//void execute_background_process(char **, char *[]);

int main(int argc, char *argv[], char *envp[])
{
  //environ = envp;
  int n;
  if(argc == 1)
  {
    //char *shell_cmd;
    
    //int i;
    while(1) 
    {
      printf("sbush> ");
      char shell_cmd_cpy[1024];
      n = read(0, shell_cmd_cpy, sizeof shell_cmd_cpy);
      if(n < 0)
      {
        puts("read error");
      }
     
      if(strncmp(shell_cmd_cpy,"cd", 2) == 0)
      {
        //execute_builtin(shell_cmd_cpy);
      }
      else if(strncmp(shell_cmd_cpy,"exit", 4)==0)
      {
        //execute_builtin(shell_cmd_cpy);
      }
      else
      {
        char **shell_cmds;
        shell_cmds = tokenize_string(shell_cmd_cpy);
        //printf("%s %s\n", *shell_cmds[0], *shell_cmds[1]);
        execute(shell_cmds); 
      }
      
    }
  }						
}

void execute(char **shell_cmds)
{
  pid_t cid;
  //char *argv[64];
  int status;
  char *const envp[] = {"/bin:", NULL};
  char *argv[32];
  int i=0;
  while(shell_cmds[i] != NULL)
  {
    //puts(shell_cmds[i]);
    argv[i] = shell_cmds[i];
    i++;
  }
  //printf("i is %d\n", i);
  argv[i] = NULL;
  if(strcmp(argv[0], "export") == 0)
  {
    char *cmd;
    char *cmd1;
    cmd = strtok(argv[1], "= \n\t");
    cmd1 = strtok(NULL, "= \n\t");
    i =0;
    while(*(envp + i) != NULL)
    {
      if(*(envp + i) == cmd)
      {
        int status = setenv(cmd, cmd1, 1);
        if(status == -1) exit(EXIT_FAILURE);
      }
      i++;
    }
  }     
  else
  {
    cid = fork();
    if(cid == 0)
    {
      //printf("token cmds inside fork is %s", argv[0]);
      if(strncmp(argv[0], "ps", strlen("ps")) == 0)
      {
        execve("/bin/ps"/*token_cmds[0]*/, argv, envp);    /* use execvpe here */
      }
      else if(strncmp(argv[0], "ls", strlen("ls")) == 0)
      {
        execve("/bin/ls", argv, envp);
      }
      else if(strncmp(argv[0], "echo", strlen("echo")) == 0)
      {
        execve("/bin/echo", argv, envp);
      }
      else if(strncmp(argv[0], "cat", strlen("cat")) == 0)
      {
        execve("/bin/cat", argv, envp);
      }
      else if(strncmp(argv[0], "kill", strlen("kill")) == 0)
      {
        execve("/bin/kill", argv, envp);
      }
      else if(strncmp(argv[0], "sleep", strlen("sleep")) == 0)
      {
        //execve("/bin/sleep", argv, envp);
        execve("/bin/sleep", argv, envp);
        //exit(EXIT_SUCCESS);
      }
      else if(strncmp(argv[0], "preempt", strlen("preempt")) == 0)
      {
        execve("/bin/preempt", argv, envp);
      }
      else
      {
        printf("This command not supported by BLEH\n");
      }
      exit(EXIT_FAILURE);
    } 
    else if (cid > 0) 
    {
      int w;
      do
      {
        //printf("Parent about to wait  ");
        w = wait(&status);
        printf("status is %d\n", status);
	if(w != cid)
        {
          exit(EXIT_FAILURE);
	}
	else
        {
	  if(status == 9)
	    puts("Child was killed by SIGTERM");
	  else if (status == 11)
	    puts("Child was killed by the SIGSEGV signal");
	}
	            
      } while(!(status == 9 || status == 11 || status == 0 || status == 1));
    } 
    else 
    {
      puts("fork error");
    }
  }
}

/*
void execute_builtin(char** token_cmds)
{
  if(strcmp(token_cmds[0],"cd") == 0)
  {
    int cd_return;
    if(token_cmds[1] != NULL)
    {
      cd_return = chdir(token_cmds[1]);
      if(cd_return == -1) exit(EXIT_FAILURE);
      else exit(EXIT_SUCCESS);
    }
  }	
}
*/

void execute_background_process(char** token_cmds, char* envp[])
{
  pid_t cid;
  cid = fork();
  char *argv[64];
  int i=0;
  if(cid == 0)
  {
    while(token_cmds[i] != NULL)
    {
      if(strchr(token_cmds[i], '&') != 0)
      {
	argv[i] = token_cmds[i];
	i++;
      }	
     }
     argv[i] = NULL;
     //printf("envp is %x", envp);
     execve(argv[0], argv, envp);
     //printf("Error while executing cmd\n");
     exit(EXIT_FAILURE);	
  } 
  else if (cid < 0)
  {
    puts("Fork error\n");
  }
}

/*
void execute_pipe(char** token_cmds, int pos, char* envp[])
{
  pid_t cid, cid1;
  int filedes[2];
  //int status;
  if(pipe(filedes) == -1) exit(EXIT_FAILURE);
        
  cid = fork();
  if(cid == -1) exit(EXIT_FAILURE);
  else if(cid == 0)
  {
    char *argv[64];
    close(filedes[1]);
    if(dup2(filedes[0],0) == -1) exit(EXIT_FAILURE);
    close(filedes[0]);
		
		int i = 0;
		while(token_cmds[i + (pos)] != NULL){
			argv[i] = token_cmds[i + (pos)];
                        puts(argv[i]);
			i++;
		} 
		argv[i] = NULL;
                
                
		execvpe(argv[0], argv, envp);
		exit(EXIT_FAILURE);
	}
	cid1 = fork();
	if(cid1 == -1) exit(EXIT_FAILURE);
	else if(cid1 == 0){
		char *argv[64];
                close(filedes[0]);
                if(dup2(filedes[1],1) == -1) {
                 
                 exit(EXIT_FAILURE);
                }
		
                close(filedes[1]);
                
		int i = 0;
                
		while(i < (pos-1)){
			argv[i] = token_cmds[i];
                        puts(argv[i]);
			i++;
		}
		argv[i] = NULL;
                
                
		execvpe(argv[0], argv, envp);
               
		exit(EXIT_FAILURE);
	} 
	close(filedes[0]);
	close(filedes[1]);
	while(waitpid(-1, NULL, 0) > 0);
}

char** tokenize_string_with_pipe(char string[], int* pos){
	if(!string)
		return 0;
	char** shell_cmds;
	char* cmd;
	int index=0, size_of_cmd=64;
	shell_cmds=malloc(size_of_cmd * sizeof(char*));
	cmd = strtok(string, " \t\n");
        puts(cmd);
	while(cmd != NULL){
		if(strcmp(cmd, "|") == 0){
			*pos = index + 1; // position of next command
		} 
		shell_cmds[index++] = cmd;
		cmd = strtok(NULL, " \t\n");
                
                
	}
	shell_cmds[index] = NULL;
	return shell_cmds;
}
*/

char* collect_shell_command(){
  int c, size=0;
  size_t size_of_buffer = 1024;
  char* buffer;
  buffer = (char*) malloc(size_of_buffer * sizeof(char));
  while(1)
  {
    c = getchar();
    //puts("getchar return");		
    if(c == EOF || c == '\n')
    {
	buffer[size] = '\0';
	break;
    }
    buffer[size++] = c;
  }
  puts("return buffer");
  return buffer;
}

/*
char *collect_shell_command_from_file(FILE *fp){
   int c, size = 0;
   size_t size_of_buffer = 1024;
   char *buffer;
   buffer = (char *)malloc(size_of_buffer * sizeof(char));
   while(1){
       c = fgetc(fp);
       if(c == EOF || c == '\n'){
          buffer[size] = '\0';
          break;
       }
       buffer[size++] = c;
   }
   return buffer;
}*/

int is_whitespace(unsigned char c)
{
  if ( c == ' '
    || c == '\f'
    || c == '\n'
    || c == '\r'
    || c == '\t'
    || c == '\v' ) 
      return 1;

  return 0;
}

void clear_array(char p[], int size)
{
  int i = 0;
  for(; i < size; i++) *(p+i) = '\0';
}


char **tokenize_string(char string[])
{
  char *temp = string;
  char cmd[64];
  char *dst;
  char **cmds = malloc(32 * sizeof(uint64_t));
  clear_array(cmd, 64);
  int index = 0, is_found = 0, is_prev_found = 0, i = 0;
  for(; *temp != '\0'; temp++)
  {
   
    is_found = (is_whitespace(*temp)) ? 0 : 1;
    if(is_found)
    {
      is_prev_found = is_found;
      cmd[i++] = *temp;
    }
    else
    {
      if(is_prev_found == 1)
      {
        cmd[i] = '\0';
        i=0;
        dst = malloc(64);
        while(cmd[i] != '\0')
        {
          dst[i] = cmd[i];
          i++;
        }
        
        i=0;
        cmds[index++] = dst;
        clear_array(cmd, 64);
        is_found = 0;
        is_prev_found = 0;
      }
      else is_found = 0;
    }
  }
  cmd[i] = '\0';
  cmds[index] = NULL;
  return cmds;
}

