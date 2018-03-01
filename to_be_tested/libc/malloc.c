#include <sys/defs.h>

/* included from asm-generic */

#define PROT_READ       0x1             /* page can be read */
#define PROT_WRITE      0x2             /* page can be written */
#define MAP_PRIVATE     0x02            /* Changes are private */
#define MAP_ANONYMOUS   0x20            /* don't use a file */



void *mmap(void *addr, size_t len, int prot, int flags, int fildes, int off);

void *malloc(size_t size){

  void *ram;
  ram = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
  return ram;
}
