#ifndef __KPRINTF_H
#define __KPRINTF_H

extern unsigned short *vgamemory;
extern unsigned int x;
extern unsigned int y;

extern int color;

void kputchar(const char );

void kputs(const char *);

void kprintf(const char *fmt, ...);

void convert_to_base(unsigned int, int, char *);

char *itoa(unsigned int );

void set_x(unsigned int );

void set_y(unsigned int );

//int my_strlen(const char *);

unsigned int get_x();

unsigned int get_y();

void my_strcpy(char *s, const char *t);

#endif
