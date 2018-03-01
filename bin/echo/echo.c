/*** echo implementation ***/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv, char **envp)
{
  int index;
  int is_first = 1;
  if(argc == 1)
  { 
    puts("\n");
    return EXIT_SUCCESS;
  }
  else if(argc > 1)
  {
    if(strcmp(argv[1], "-n") == 0)
    {
      /* do not have any trailing newline */
      index = 2;
      if(is_first)
      {
        
        printf(" ");
        is_first = 0;
      }
      for(; index < argc; index++)
        printf("%s ", argv[index]);
    }
    else
    {
      index = 1;
      
      if(is_first)
      {
        printf(" ");
        is_first = 0;
      }
      for(; index < argc; index++)
        printf("%s ", argv[index]);
      printf("\n");
    }
  }
  else {puts("usage : echo ""string""\n"); return EXIT_FAILURE;}
  return EXIT_SUCCESS;
}
