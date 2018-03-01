// memcpy.c implementation

void memcpy(void *dest, const void *src, int n){
	char *dest1 = (void *)dest;
	char *src1 = (void *)src;
	int i = 0;
	for(i = 0; i < n; i++){
		dest1[i] = src1[i];
	}
}
