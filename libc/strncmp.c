
int strncmp(char *cs, char *ct, int n){
  char *p1 = cs;
  char *p2 = ct;
  int i = 0;
  int n1 = 0;
  while(*(p1 + i) != '\0' && n1 < n ){
    if(*(p2 + i) == '\0') return 1;
    else if(*(p1 + i) < *(p2 + i)) return -1;
    else if(*(p1 + i) > *(p2 + i)) return 1;
    else{
      i++;
      n1++;
    }
  }
  if(*(p2 + i) != '\0') {
    if(*(p1 + i) == '\0') return -1;  // cs is smaller
  }
  return 0;
}
