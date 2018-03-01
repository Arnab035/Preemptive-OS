#include <stdio.h>
#include <unistd.h>

char *fgets(char *dest, int n, FILE *fp)
{
	if(fp == NULL) return NULL;
        char *p = dest;
        char c;
        while(n > 0){
                c = fgetc(fp);
		putchar(c);
                if(c == EOF){
                        if(p == dest) return NULL;
			else break;
		}
		if(c == '\n') break;
                *p++ = c;
                n--;
        }
        *p = '\0';
        return p;

}