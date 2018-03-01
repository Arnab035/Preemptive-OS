

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

/*
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
*/

char** environ;

char* collect_shell_command();
char* collect_shell_command_from_file(FILE *);
char** tokenize_string(char* );
void execute_pipe(char** , int , char*[] );
char** tokenize_string_with_pipe(char* , int* );
void execute(char** ,char*[]);
void execute_builtin(char**);
void execute_background_process(char **, char *[]);

int main(int argc, char *argv[], char *envp[]){
	environ = envp;
	if(argc == 1){
                char *shell_cmd;
		
		char** shell_cmds;
		int pos;
                int i, found_and, found_pipe;
		while(1) {
			puts("sbush> ");
                        char shell_cmd_cpy[1024];
			shell_cmd = collect_shell_command();
                        found_pipe = 0;
                        found_and = 0;
                        i = 0;
                        
                        for(;*(shell_cmd) != '\0'; shell_cmd++){
                        	shell_cmd_cpy[i] = *shell_cmd;
                                
                                i++; 
                        }
                        shell_cmd_cpy[i] = '\0';  
                        for(i=0; shell_cmd_cpy[i] != '\0'; i++){
				if(shell_cmd_cpy[i] == '|'){
					found_pipe = 1;
                                }
                                if(shell_cmd_cpy[i] == '&'){
					found_and = 1;
                                }
                        }
			if(found_pipe == 1){
                        	shell_cmds = tokenize_string_with_pipe(shell_cmd_cpy, &pos);
				
				execute_pipe(shell_cmds, pos, envp);
                        } else if(found_and == 1){

                        	shell_cmds = tokenize_string(shell_cmd_cpy);
				
				execute_background_process(shell_cmds, envp);
                        } else {
				shell_cmds = tokenize_string(shell_cmd_cpy);
				
				if(strcmp(shell_cmds[0],"cd") == 0 || (strcmp(shell_cmds[0],"exit")==0)){
                    			execute_builtin(shell_cmds);
				}else{
                                        
					execute(shell_cmds, envp);
                                        
				}
			}
		} 
	}
        else{
            const char *filename;
	    filename = argv[1];
            FILE *fp = fopen(filename, "r");
            puts("open");
            if(fp == NULL) exit(EXIT_FAILURE);
            int pos;
            char *shell_cmd = (char *)malloc(1024 * sizeof(char));
            char **shell_cmds;
            int found_pipe;
            int found_and;
            int i;
            while(fgets(shell_cmd, 1024, fp)){
              
	      puts(shell_cmd);
              if(strchr(shell_cmd, '#') != NULL) continue;
              if(shell_cmd[0] == '\n') continue;
              
              char shell_cmd_cpy[1024];
              i = 0;
              for(;*(shell_cmd) != '\0'; shell_cmd++){
                        shell_cmd_cpy[i] = *shell_cmd;
                                
                        i++; 
              }
              shell_cmd_cpy[i] = '\0'; 
              found_pipe = 0;
              found_and = 0; 
            for(i=0; shell_cmd_cpy[i] != '\0'; i++){
		    	if(shell_cmd_cpy[i] == '|'){
					found_pipe = 1;
             	}
            	if(shell_cmd_cpy[i] == '&'){
					found_and = 1;
               	}
            }
            if(found_pipe == 1){
                    shell_cmds = tokenize_string_with_pipe(shell_cmd_cpy, &pos);
					execute_pipe(shell_cmds, pos, envp);
             } else if(found_and == 1){

                        shell_cmds = tokenize_string(shell_cmd_cpy);
						execute_background_process(shell_cmds, envp);
               } else {
						shell_cmds = tokenize_string(shell_cmd_cpy);
				
						if(strcmp(shell_cmds[0],"cd") == 0 || (strcmp(shell_cmds[0],"exit")==0)){
                    		execute_builtin(shell_cmds);
			}else{
                        execute(shell_cmds, envp);
                                        
			}
		}
           }

          int clret = fclose(fp);
          if(clret == -1) exit(EXIT_FAILURE);
         
        }
	return 1;
}

void execute(char** token_cmds, char* envp[]){
	pid_t cid;
	char *argv[64];
	int status;
	if(strcmp(token_cmds[0], "export") == 0){
           char *cmd;
           char *cmd1;
           cmd = strtok(token_cmds[1], "= \n\t");
           cmd1 = strtok(NULL, "= \n\t");
           int i =0;
           while(*(envp + i) != NULL){
           	  if(*(envp + i) == cmd){
           	  	// setenv(name , value, overwrite)
           	  	int status = setenv(cmd, cmd1, 1);
           	  	if(status == -1) exit(EXIT_FAILURE);
           	  }
           	  i++;
           }
          
    } else{
		cid = fork();
		if(cid == 0){
			int i = 0;
			while(token_cmds[i] != NULL){
				argv[i] = token_cmds[i];
                                
				i++;
                                
			}
			argv[i] = NULL;
                        
			execvpe(argv[0], argv, envp);
			
			exit(EXIT_FAILURE);
		} else if (cid > 0) {
			int w;
			do{
				w =  waitpid(cid, &status, 0);
				if(w != cid){
					//printf("are you here?\n");
					exit(EXIT_FAILURE);
				}
				if(w == cid) {
					if (WIFEXITED(status))
	                 		puts("Child ended normally");
	            			else if (WIFSIGNALED(status))
	                 		puts("Child ended because of an uncaught signal");
	            	
	                //exit(EXIT_SUCCESS);
				}
	            
	        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
		} else {
			//printf("fork error");
		}
	}
}

void execute_builtin(char** token_cmds){
	if(strcmp(token_cmds[0],"cd") == 0){
		int cd_return;
		if(token_cmds[1] != NULL){
			cd_return = chdir(token_cmds[1]);
			if(cd_return == -1){
				exit(EXIT_FAILURE);
			}
		}
	} else {
		exit(EXIT_SUCCESS);
	}	
}

void execute_background_process(char** token_cmds, char* envp[]){
	pid_t cid;
	cid = fork();
	char *argv[64];
	int i=0;
	if(cid == 0){
		while(token_cmds[i] != NULL){
			if(strchr(token_cmds[i], '&') != 0){
				argv[i] = token_cmds[i];
				i++;
			}	
		}
		argv[i] = NULL;
		execvpe(argv[0], argv, envp);
		//printf("Error while executing cmd\n");
		exit(EXIT_FAILURE);	
	} else if (cid < 0){
		//printf("Fork error\n");
	}
}


void execute_pipe(char** token_cmds, int pos, char* envp[]){
	pid_t cid, cid1;
	int filedes[2];
	//int status;
	if(pipe(filedes) == -1) exit(EXIT_FAILURE);
        
	cid = fork();
	if(cid == -1) exit(EXIT_FAILURE);
	else if(cid == 0){
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


char* collect_shell_command(){
	int c, size=0;
	size_t size_of_buffer = 1024;
	char* buffer;
	buffer = (char*) malloc(size_of_buffer * sizeof(char));
	while(1){
		c = getchar();
		
		
		if(c == EOF || c == '\n'){
			buffer[size] = '\0';
			break;
		}
		buffer[size++] = c;
	}
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


char** tokenize_string(char string[]){
	if(!string)
		return 0;
	char** shell_cmds;
	char* cmd;
        
	int index=0, size_of_cmd=64;
	shell_cmds = malloc(size_of_cmd * sizeof(char*));
	cmd = strtok(string, " \t\n");
	
	while(cmd != NULL){
		shell_cmds[index++] = cmd;
		cmd = strtok(NULL, " \t\n");
		
		//puts(cmd);
	}
	
	shell_cmds[index] = NULL;
	return shell_cmds;
}
