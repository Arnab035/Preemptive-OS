#ifndef _ELF64_H
#define _ELF64_H

#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/vfs.h>
#include <sys/memory.h>    // header file containing functions related to mmaps

#define EI_NIDENT 16

typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Lword;
typedef uint64_t Elf64_Off;
typedef uint32_t Elf64_Sword;
typedef uint64_t Elf64_Sxword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;

typedef struct {
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half    e_type;
  Elf64_Half    e_machine;
  Elf64_Word    e_version;
  Elf64_Addr    e_entry;
  Elf64_Off     e_phoff;
  Elf64_Off     e_shoff;
  Elf64_Word    e_flags;
  Elf64_Half    e_ehsize;
  Elf64_Half    e_phentsize;
  Elf64_Half    e_phnum;
  Elf64_Half    e_shentsize;
  Elf64_Half    e_shnum;
  Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

typedef struct {
  Elf64_Word    p_type;
  Elf64_Word    p_flags;
  Elf64_Off     p_offset;
  Elf64_Addr    p_vaddr;
  Elf64_Addr    p_paddr;
  Elf64_Xword   p_filesz;
  Elf64_Xword   p_memsz;
  Elf64_Xword   p_align;
} Elf64_Phdr;

#define ELFMAG0  0x7f
#define ELFMAG1  'E'
#define ELFMAG2  'L'
#define ELFMAG3  'F'

#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

#define ELFDATANONE  0
#define ELFDATA2LSB  1
#define ELFDATA2MSB  2  

#define EM_386       3

#define PT_NULL      0
#define PT_LOAD      1
#define PT_DYNAMIC   2
#define PT_INTERP    3
#define PT_NOTE      4
#define PT_SHLIB     5
#define PT_PHDR      6
#define PT_LOPROC    0x70000000
#define PT_HIPROC    0x7fffffff

#define PF_R	     0x4
#define PF_W	     0x2
#define PF_X	     0x1

#define ELF_READ     PF_R
#define ELF_WRITE    PF_R | PF_W      // data segment
#define ELF_EXECUTE  PF_R | PF_X      // code segment

uint16_t check_elf_magic(Elf64_Ehdr *);

uint16_t check_elf_file_validity(Elf64_Ehdr *);

Elf64_Phdr *load_elf_phdr(Elf64_Ehdr *ehdr, struct file *);

int load_elf_binary(struct mm_struct *);

Elf64_Phdr *read_elf_pheader(Elf64_Ehdr *, int );

uint64_t elf_map(struct file *, Elf64_Phdr *);



#endif
