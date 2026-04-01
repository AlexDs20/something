#include <sys/mman.h>       // mmap
#include <sys/stat.h>       // stat
#include <stdio.h>          // perror
#include <fcntl.h>          // open, O_RDONLY
#include <stdlib.h>         // exit, ...
#include <unistd.h>         // close, write

#include "memory/allocators.h"
#include "platform/io.h"

void read_n_bytes(char* filepath, unsigned int n, char* buffer) {
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

    if (sb.st_size < n) {
        fprintf(stderr, "File size smaller than requested.");
        exit(3);
    }


    int r = read(fd, buffer, n);
    if (r < 0) {
        perror("read");
        exit(4);
    }
    if ((unsigned int)r != n) {
        fprintf(stderr, "Short read.\n");
        exit(5);
    }

    close(fd);
}

String read_complete_file(Arena* arena, StringView filepath) {
    String fp = string_init_sv(arena, filepath);
    int fd = open(string_as_cstr(&fp), O_RDONLY);
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

    String out = string_init_empty(arena, sb.st_size+1);
    if (out.buffer == NULL) {
        printf("\nFailed.");
    }
    string_append_buffer(arena, &out, data, sb.st_size);
    string_append_cstr(arena, &out, "\0");

    // Print file contents
    // write(1, data, sb.st_size);

    munmap(data, sb.st_size);
    close(fd);

    return out;
}
