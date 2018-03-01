#include <unistd.h>
#include <stdio.h>

#define O_RDONLY        00000000
#define O_WRONLY        00000001


static int cat_fd(int fd) {
  char buf[4096];
  int reads;

  while ((reads = read(fd, buf, sizeof buf)) > 0) {
    int totalwritten = 0;
    while (totalwritten < reads) {
      int written = write(STDOUT_FILENO, buf + totalwritten, reads - totalwritten);  // 1 means stdout
      if (written < 1)
        return -1;
      totalwritten += written;
    }
  }

  if(reads == 0) 
	return 0;
  else return -1;
}

static int cat(const char *fname) {
  int fd, success;
  //puts(fname);
  if ((fd = open(fname, O_RDONLY, 0)) == -1)
    return -1;

  success = cat_fd(fd);

  if (close(fd) != 0)
    return -1;

  return success;
}


int main(int argc, char **argv, char **envp) {
  int i;

  if (argc == 1) {
    if (cat_fd(0) != 0)
      puts("Error");
  } else {
    for (i = 1; i < argc; i++) {
      //printf("About to open: %s\n", argv[i]);
      if (cat(argv[i]) != 0)
        puts("Error");
    }
  }
  return 0;
}
