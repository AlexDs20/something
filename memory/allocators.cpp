#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "memory/allocators.h"
#include "allocators.h"
#include "utils/types.h"
#include "platform/memory.h"

#define DEFAULT_ALIGN (sizeof(void*))                       // Bytes
#define ALIGN_TO(v, a) (((v)+((a)-1)) & (~((a)-1)))
#define ALIGN(v) ALIGN_TO((v), DEFAULT_ALIGN)

//==============================
// Arena (Linear)
Arena* arena_alloc_create(u64 capacity) {
    MemoryBlock arena_block = os_memory_alloc(sizeof(Arena));
    Arena* arena = (Arena*) arena_block.ptr;

    if (!arena) {
        return 0;
    }
    MemoryBlock buffer_block = os_memory_reserve(capacity);
    if (!buffer_block.ptr) {
        os_memory_free(arena, sizeof(Arena));
        return 0;
    }
    arena->buffer = (u8*)buffer_block.ptr;
    arena->capacity = buffer_block.size;
    arena->committed = 0;
    arena->top = 0;
    return arena;
}

Arena* arena_alloc_create_zero(u64 capacity) {
    Arena* arena = arena_alloc_create(capacity);
    if (!arena || !arena->buffer) {
        return 0;
    }
    // memset(arena->buffer, 0, arena->capacity);       // Not Needed as mmap automatically sets to 0 => use when going multi platform
    return arena;
}

u64 arena_alloc_used(Arena* arena) {
    if (!arena || !arena->buffer) {
        return 0;
    }
    return arena->top;
}

u64 arena_alloc_remaining(Arena* arena) {
    if (!arena || !arena->buffer) {
        return 0;
    }
    return arena->capacity - arena->top;
}

void* arena_alloc_get(Arena* arena, u64 pos) {
    if (!arena || !arena->buffer) {
        return 0;
    }
    if (pos > arena->top) {
        return 0;
    }
    return (void*)(arena->buffer + pos);
}

void* arena_alloc_push(Arena* arena, u64 size) {
    return arena_alloc_push_aligned(arena, size, DEFAULT_ALIGN);
};

void* arena_alloc_push_aligned(Arena* arena, u64 size, u64 alignment) {
    bool power_of_two = (alignment & (alignment-1)) == 0;
    if (!power_of_two) {
        return 0;
    }
    u64 checkpoint = arena_alloc_checkpoint(arena);
    arena->top = ALIGN_TO(arena->top, alignment);               // push_unaligned handles the overflow
    void* allocation = arena_alloc_push_unaligned(arena, size);
    if (!allocation) {
        arena_alloc_restore(arena, checkpoint);
        return 0;
    }
    return allocation;
}

void* arena_alloc_push_unaligned(Arena* arena, u64 size) {
    if (!arena || !arena->buffer) {
        return 0;
    }
    u64 needed = arena->top + size;
    if (needed > arena->capacity) {
        printf("Arena overflow. Not enough space left.\n");
        return 0;
    }
    if (needed > arena->committed) {
        u64 commit_amount = needed - arena->committed;
        MemoryBlock block = os_memory_commit(arena->buffer + arena->committed, commit_amount);
        if (!block.size) {
            return 0;
        }
        arena->committed += block.size;
    }

    void* allocation = arena->buffer + arena->top;
    arena->top += size;
    return allocation;
}

void* arena_alloc_push_zero(Arena* arena, u64 size) {
    return arena_alloc_push_zero_aligned(arena, size, DEFAULT_ALIGN);
}

void* arena_alloc_push_zero_aligned(Arena* arena, u64 size, u64 alignment) {
    bool power_of_two = (alignment & (alignment-1)) == 0;
    if (!power_of_two) {
        return 0;
    }
    u64 checkpoint = arena_alloc_checkpoint(arena);
    arena->top = ALIGN_TO(arena->top, alignment);

    void* allocation = arena_alloc_push_zero_unaligned(arena, size);
    if (!allocation) {
        arena_alloc_restore(arena, checkpoint);
        return 0;
    }
    return allocation;
}

void* arena_alloc_push_zero_unaligned(Arena* arena, u64 size) {
    void* allocation = arena_alloc_push_unaligned(arena, size);
    if (!allocation) {
        return 0;
    }
    memset(allocation, 0, size);
    return allocation;
}

void* arena_alloc_push_struct(Arena* arena, void* data, u64 size) {
    return arena_alloc_push_struct_aligned(arena, data, size, DEFAULT_ALIGN);
}

void* arena_alloc_push_struct_aligned(Arena* arena, void* data, u64 size, u64 alignment) {
    bool power_of_two = (alignment & (alignment-1)) == 0;
    if (!power_of_two) {
        return 0;
    }
    u64 checkpoint = arena_alloc_checkpoint(arena);
    arena->top = ALIGN_TO(arena->top, alignment);

    void* allocation = arena_alloc_push_struct_unaligned(arena, data, size);
    if (!allocation) {
        arena_alloc_restore(arena, checkpoint);
        return 0;
    }
    return allocation;
}

void* arena_alloc_push_struct_unaligned(Arena* arena, void* data, u64 size) {
    void* allocation = arena_alloc_push_unaligned(arena, size);
    if (!allocation) {
        return 0;
    }
    memcpy(allocation, data, size);
    return allocation;
}

void arena_alloc_pop_by(Arena* arena, u64 size) {
    if(!arena || size > arena->top) {
        printf("Invalid pop from arena.\n");
        return;
    }
    arena->top -= size;
}

void arena_alloc_pop_by_zero(Arena* arena, u64 size) {
    if(!arena || size > arena->top) {
        printf("Invalid pop from arena.\n");
        return;
    }
    arena->top -= size;
    memset(arena->buffer + arena->top, 0, size);
}

void* arena_alloc_pop_to(Arena* arena, void* addr) {
    if (!arena || !arena->buffer) {
        return 0;
    }
    if ((u64)addr < (u64)arena->buffer || (u64)addr>=(u64)(arena->buffer+arena->top)) {
        return 0;
    }

    arena->top = (u64)addr - (u64)arena->buffer;
    return (void*)(arena->buffer + arena->top);
}

void* arena_alloc_pop_to_zero(Arena* arena, void* addr) {
    u64 checkpoint = arena_alloc_checkpoint(arena);
    void* ret = arena_alloc_pop_to(arena, addr);
    if (!ret) {
        return 0;
    }
    memset(arena->buffer + arena->top, 0, checkpoint - arena->top);
    return ret;
}

void arena_alloc_free(Arena* arena) {
    if (arena) {
        if (arena->buffer) {
            os_memory_free(arena->buffer, arena->capacity);
        }
        os_memory_free(arena, sizeof(Arena));
    }
}

void arena_alloc_reset(Arena* arena) {
    if (arena) {
        arena->top = 0;
    }
}

void arena_alloc_reset_zero(Arena* arena) {
    if (arena) {
        if (arena->buffer) {
            memset(arena->buffer, 0, arena->top);
        }
        arena->top = 0;
    }
}

void* arena_alloc_copy(Arena* dest, Arena* src) {
    if (!src || !dest || (dest == src)) {
        return 0;
    }
    void* new_alloc = arena_alloc_push(dest, src->top);
    if (!new_alloc) {
        return 0;
    }
    void* alloc = memcpy(new_alloc, (void*)src->buffer, src->top);
    return alloc;
}

u64 arena_alloc_checkpoint(Arena* arena) {
    return arena ? arena->top : 0;
}

void arena_alloc_restore(Arena* arena, u64 checkpoint) {
    if (arena && checkpoint <= arena->top) {
        arena->top = checkpoint;
    }
}

void arena_debug_print(Arena* arena) {
    if (!arena) return;
    printf("Arena Debug Info:\n");
    printf("  buffer:     %p\n", arena->buffer);
    printf("  capacity:   %lu bytes\n", arena->capacity);
    printf("  committed:  %lu bytes\n", arena->committed);
    printf("  used:       %lu bytes\n", arena->top);
    printf("  free:       %lu bytes\n", arena->capacity - arena->top);
}

void arena_debug_map(Arena* arena, u64 width) {
    if (!arena) return;

    const u64 total = arena->capacity;
    const u64 committed = arena->committed;
    const u64 used = arena->top;

    for (u64 i = 0; i < width; i++) {
        u64 offset = (i * total) / width;
        char symbol = '.';
        if (offset < used) {
            symbol = '#';
        } else if (offset < committed) {
            symbol = '~';
        }
        putchar(symbol);
    }
    putchar('\n');
}

//==============================
// Vector
Vector* vector_alloc_create(u64 n, u64 size) {
    Vector* vector = (Vector*)malloc(sizeof(Vector));
    if (!vector) {
        printf("Could not allocate memory for the vector structure.\n");
        return 0;
    }

    vector->buffer = (u8*)malloc(n*size);
    if(!vector->buffer) {
        printf("Could not allocate memory for the vector buffer.\n");
        free(vector);
        return 0;
    }
    vector->capacity = n*size;
    vector->top = 0;
    vector->element_size = size;
    return vector;
}

void* vector_alloc_push(Vector* vector, void* data) {
    if (!vector) {
        return 0;
    }
    if (vector->top+vector->element_size > vector->capacity) {
        u8* new_buffer = (u8*)malloc(2*vector->capacity);
        if (!new_buffer) {
            printf("could not expand memory buffer\n");
            return 0;
        }
        memcpy(new_buffer, vector->buffer, vector->top);
        free(vector->buffer);
        vector->capacity *= 2;
        vector->buffer = new_buffer;
    }
    void* allocated = memcpy((void*)(vector->buffer + vector->top), data, vector->element_size);
    vector->top += vector->element_size;
    return allocated;
}

void vector_alloc_pop(Vector* vector) {
    if (vector) {
        if (vector->buffer && vector->top>=vector->element_size) {
            vector->top -= vector->element_size;
            memset((void*)(vector->buffer + vector->top), 0, vector->element_size);
        }
    }
}

void vector_alloc_free(Vector* vector) {
    if (vector) {
        if (vector->buffer) {
            free(vector->buffer);
        }
        free(vector);
    }
}

void vector_alloc_trim(Vector* vector) {
    if (vector->top == vector->capacity) {
        return;
    }
    u8* new_buffer = (u8*)malloc(vector->top);
    memcpy((void*)new_buffer, (void*)vector->buffer, vector->top);
    free(vector->buffer);
    vector->buffer = new_buffer;
    vector->capacity = vector->top;
}

u64 vector_alloc_count(Vector* vector) {
    if (vector) {
        return vector->top / vector->element_size;
    }
    return 0;
}

void* vector_alloc_get(Vector* vector, u64 index) {
    if (index*vector->element_size < vector->top) {
        return vector->buffer + index*vector->element_size;
    }
    return 0;
}

void* vector_alloc_set(Vector* vector, u64 index, void* data) {
    if (index<vector->top) {
        memcpy((void*)(vector->buffer + index*vector->element_size), data, vector->element_size);
        return vector->buffer + index*vector->element_size;
    }
    return 0;
}
//==============================

