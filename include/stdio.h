#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <sys/defs.h>

typedef struct file{
   int fd;
} FILE;

static const int EOF = -1;
int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);
FILE *fopen(const char *filename, const char *mode);
char *fgets(char *dest, int n, FILE *fp);
size_t vprintf(char *, size_t, const char *, va_list);
int fgetc(FILE *fp);
int getchar();
int fclose(FILE *fd);

char *gets(char *s);

#endif
