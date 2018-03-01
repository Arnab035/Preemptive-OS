#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int fclose(FILE *fp){
	int status;
	status = close(fp->fd);
	if(status == 0){
		// successful close
		int s = my_munmap(fp, sizeof(FILE));
		if(s == -1) return -1;
	}
	return status;
}
