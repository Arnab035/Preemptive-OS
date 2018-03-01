
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// basic open functionality -- no create mechanism -- HANDLE READ AND WRITE !

#define O_RDONLY        00000000
#define O_WRONLY        00000001
#define S_IRUSR         00400
#define S_IXUSR         00100
#define S_IRGRP         00040
#define S_IXGRP		00010
#define S_IROTH		00004
#define S_IXOTH		00001



FILE *fopen(const char *filename, const char *mode){
	if(filename == NULL) return NULL;
	int fd;
	if(strchr(mode,'r') == NULL && strchr(mode, 'w') == NULL) return NULL;
	FILE *fp;
	if(strchr(mode,'r') != NULL){
		fd = open(filename, O_RDONLY, 0);
		fp->fd = fd;
	} else if(strchr(mode, 'w') != NULL){
		fd = open(filename, O_WRONLY, 0);
		fp->fd = fd;
	}
	
    
    return fp;
}
