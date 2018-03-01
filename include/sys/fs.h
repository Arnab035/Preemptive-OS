#ifndef __FS_H
#define __FS_H

#include <sys/defs.h>
#include <sys/stat.h>
#include <sys/process.h>
#include <sys/tarfs.h>
#include <sys/vfs.h>
#include <sys/terminals.h>

#define FD_MAX   256

struct pcb *processes;

struct file *fp;

int sys_open(const char *pathname, int flags, mode_t mode);

int tarfs_open(char *pathname, int flags, mode_t mode);

int tarfs_close(struct file *);

ssize_t sys_read(int, void *, size_t);

ssize_t sys_write(int, void *, size_t);

int sys_close(int );

struct file *do_open_exec(const char *, int , int );

int available_fd(struct pcb *);

#endif
