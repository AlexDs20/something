#include <stdio.h>
#include <string.h>
#include "utils/defines.h"
#include "platform/io.h"
#include "errno.h"
#include "libs/libstring.h"

#include <sys/stat.h>       // stat
#include <stdio.h>          // perror
#include <fcntl.h>          // open, O_RDONLY
#include <stdlib.h>         // exit, ...
#include <unistd.h>         // close, write

string8 read_file(Arena* arena, string8 file_path) {
    LocalArena* local_arena = local_arena_alloc_create();

    char* cstr = string_to_cstr(local_arena->arena, file_path);
    FILE* file = fopen(cstr, "r");

    string8 file_content = {0};

    if (!file) {
        printf("fopen %s failed: %s\n", cstr, strerror(errno));
        local_arena_alloc_reset(local_arena);
        return file_content;
    }

    u64 checkpoint = arena_alloc_checkpoint(arena);

    u64 chunk_size = 4*KiB;
    u64 bytes_read = 0;
    u8* first_loc = (u8*)arena_alloc_align(arena);

    do {
        void* loc = arena_alloc_push_unaligned(arena, chunk_size);
        bytes_read = fread(loc, 1, chunk_size, file);
        if (bytes_read < chunk_size) {
            if (ferror(file)) {
                printf("Failed while reading the file: %s!\n", cstr);
                arena_alloc_restore(arena, checkpoint);
                return file_content;
            }
            arena_alloc_pop_by(arena, chunk_size - bytes_read);
        }

    } while (bytes_read == chunk_size);

    int error = fclose(file);
    if (error) {
        printf("fclose failed: %s\n", strerror(errno));
        arena_alloc_restore(arena, checkpoint);
        local_arena_alloc_reset(local_arena);
        return file_content;
    }

    file_content.buffer = first_loc;
    file_content.size = (u64)((arena->buffer+arena->top) - first_loc);

    local_arena_alloc_reset(local_arena);
    return file_content;
}

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
