// all string functions coming up

#ifndef _STRING_H
#define _STRING_H

#include <sys/defs.h>

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
char *strtok(char s1[], char *s2);
int strlen(const char* s);
void memcpy(void *dest, const void *src, int n);
char *strcpy(char *dest, char *src);

#endif
