#include <string.h>
#include <sys/defs.h>
#include <stdlib.h>

extern char *getenv(const char *name);

extern char **environ;

// to be implemented

int getenv_at(const char *name){
  if(name == NULL) return -1;
  const char *envname;
  int len;
  char *c;
  char **p;
  
  for(envname = name; *envname && *envname != '='; envname++) continue;
  len = envname - name;
  for(p=environ; *p != NULL ;p++){
      c = *p;
      if(c[len] == '=' && (strncmp(c, name, len)==0)){
        return p-environ; // return position of envvar
      }
    }
  return -1;
}

int setenv(const char *name, const char *value, int overwrite){
  if(name == NULL || strchr(name,'=') == 0){ 
     return -1;
  }
  const char *envname = getenv(name);
  if(envname != NULL && overwrite == 0){
    return 0;
  }
  char *envstr;
  int len = strlen(name) + 1 + strlen(value) + 1;  // for name=value
  envstr = malloc(len * sizeof(char));
  memcpy(envstr, name, strlen(name));
  envstr[strlen(name)] = '=';
  memcpy(envstr + strlen(name) + 1, value, strlen(value));
  envstr[len-1] = '\0';
  int append = 1;
  int i;
  if((i = getenv_at(name)) >= 0){
    *(environ + i) = envstr;
    append = 0;
    //printf("Env variable added\n");
  }
  if(append){
    int len = sizeof(environ)/ sizeof(environ[0]) ;
    *(environ + len) = envstr;
    *(environ + len + 1) = NULL;
  }
  return 0;
}
