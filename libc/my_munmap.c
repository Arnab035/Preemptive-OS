#include <stdlib.h>


static inline long syscall2(long syscall, long arg1, long arg2){
	long ret;
	__asm__ __volatile__("syscall" : "=a"(ret): "a"(syscall),"D"(arg1),"S"(arg2) : "rcx","r11","memory");
	return ret;
}

int my_munmap(void *dir, size_t size){
	if(dir == NULL){
		return -1;
	}
	return syscall2((long)13, (long)dir, size);  // call munmap
}
