
// char *strchr(char *cs, char c)

#include <string.h>

char *strchr(const char *cs, int c){
  if(!cs)
    return NULL;
  int i;
  const char* temp;
  for(i = 0; *(cs + i) != '\0'; i++){
    if(*(cs + i) == c){
     temp = cs + i ;
     return (char *)temp;
    } 
  }
  return NULL;
}
