#include <stdio.h>
#include <sys/defs.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[], char* envp[]) 
{
    int j;
    j = fork();
    //List all processes in ready queue
    //ps();
    if(j==0) { 
        printf("I am a child\n");
        j = fork();
        if(j == 0){
          printf("I am a child\n");
          exit(EXIT_SUCCESS);
        }
        else
        {
          printf("I am a parent\n");
          exit(EXIT_SUCCESS);
        }   
        exit(EXIT_SUCCESS);
    }
    else {
        printf("I am a parent\n"); 
        exit(EXIT_SUCCESS);
    }  
    exit(EXIT_SUCCESS);
}
