
#include <string.h>
#include <stdio.h>

extern char **environ;

char *getenv(const char *name){
  if(name == NULL) return NULL;
 
  const char *envname;
  
  int len;
  char *c;
  char **p;
  
  for(envname = name; *envname && *envname != '='; ++envname) continue;
  
  len = envname - name;
  
  for(p=environ; *p != NULL ;++p){
      c = *p;
      
      if(c[len] == '=' && (strncmp(c, name, len)==0)){
        
        return c + len + 1; // exclude =
      }
    }
  return NULL;
}

