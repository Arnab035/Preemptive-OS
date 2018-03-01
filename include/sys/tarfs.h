#ifndef _TARFS_H
#define _TARFS_H

#define MAX_DENTRIES_IN_CACHE   512   /* 512 * 8 = 4096 */

#include <sys/defs.h>
#include <sys/vfs.h>
#include <sys/util.h>
#include <sys/process.h>
#include <sys/fs.h>
#include <sys/elf64.h>
#include <sys/mmgr.h>

extern char _binary_tarfs_start;
extern char _binary_tarfs_end;

struct dentry *dentry;

struct super_block *tarfs_sb;
struct dentry_cache *dentry_cache_array;   // array type not work ! 

struct posix_header_ustar {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char pad[12];
};

void initialize_tarfs_sb();

void tarfs_print_all_contents();

struct name *get_parent_and_child_directories(char *);

struct inode get_inode_from_dentry(struct dentry *);

void tarfs_init();

void insert_dentry_into_dentry_cache(int , struct dentry *);

struct dentry_cache *get_file_components(char *);

struct dentry_cache *convert_filename_to_dentry_cache_entry(char *);

struct file *get_file_from_fd(int );

struct dentry *do_find_dentry(char *, int );

struct dentry *check_correct_file(char *);

struct dentry *check_dentry_cache(char *);

struct inode *get_inode(struct dentry *);

struct dentry *dentry_for_this_inode(char *);

struct dentry *get_parent_dentry(char *);

void insert_inode_into_superblock(struct inode );

struct inode *tarfs_alloc_inode();

void tarfs_destroy_inode(struct inode *);

struct dentry *create_root_dentry();

ssize_t tarfs_read(struct file *, char *, size_t, off_t);

struct file *do_open_exec(const char *, int , int );

size_t sys_getdents(int, char *, int);

struct dentry_cache return_child_dentry_cache_entry(struct dentry *);

int is_dentry_present(char *);

int bool_check_dentry_cache(char *, char *);

#endif
