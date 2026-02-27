#include "io.h"

#include <sys/mman.h>       // mmap
#include <sys/stat.h>       // stat
#include <stdio.h>          // perror
#include <fcntl.h>          // open, O_RDONLY
#include <stdlib.h>         // exit, ...
#include <unistd.h>         // close, write


typedef struct {
    char* data;
    size_t size;
} string8;

void read_complete_file(char* filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
      perror("open");
      exit(1);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
      perror("fstat");
      exit(2);
    }

    char* data = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if(data == MAP_FAILED) {
      close(fd);
      perror("mmap");
      exit(3);
    }

    // Print file contents
    write(1, data, sb.st_size);

    munmap(data, sb.st_size);
    close(fd);
}
