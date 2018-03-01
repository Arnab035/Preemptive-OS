
int strlen(const char* s){
  int len = 0;
  int i = 0;
  while(*(s + i) != '\0'){
    len += 1;
    i += 1;
  }
  return len;
}
