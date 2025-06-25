#include <stdio.h>
#include <string.h>
#include "utils/defines.h"
#include "platform/io.h"
#include "errno.h"
#include "libs/libstring.h"

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
