#include <dirent.h>

int my_munmap(void *dir, size_t size);

int close(int fp);

int closedir(DIR *dir){
	int fd;
	if(dir == NULL) return -1;
	fd = dir -> fd;
	int s = my_munmap((void *)dir, sizeof(DIR));
        if(s == -1) return -1;
	return close(fd);
}