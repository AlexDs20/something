#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils/defines.h"
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
        // TODO
        if (arena->resize_method == ARENA_RESIZE_COPY) {
        } else if (arena->resize_method == ARENA_RESIZE_CHAIN) {
        } else {
            printf("Arena overflow. Not enough space left.\n");
            return 0;
        }
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

void* arena_alloc_align(Arena* arena) {
    if (!arena || !arena->buffer) {
        return 0;
    }
    return arena_alloc_push(arena, 0);
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

#if not defined(LOCAL_ARENA_POOL_COUNT)
#define LOCAL_ARENA_POOL_COUNT 10
#endif
#define LOCAL_ARENA_CAPACITY (1*GiB)
global LocalArena local_arena_pool[LOCAL_ARENA_POOL_COUNT] = {0};

LocalArena* local_arena_alloc_create() {
    LocalArena* out = 0;
    for (u64 i=0; i<LOCAL_ARENA_POOL_COUNT; i++) {
        // Initialize the local arena if it hasn't been done yet
        if (local_arena_pool[i].arena == 0) {
            local_arena_pool[i].arena = arena_alloc_create(LOCAL_ARENA_CAPACITY);
        }
        if (!local_arena_pool[i].used) {
            out = &local_arena_pool[i];
            break;
        }
    }
    if (out) {
        out->used = 1;
    } else {
        printf("No more local arenas available. Used: %d of %d.\n", LOCAL_ARENA_POOL_COUNT, LOCAL_ARENA_POOL_COUNT);
    }
    return out;
}

void local_arena_alloc_reset(LocalArena* local_arena) {
    local_arena->used = 0;
    arena_alloc_reset_zero(local_arena->arena);
}

//==============================
// Vector

Vector* vector_alloc_create(Arena* arena, u64 element_size) {
    u64 checkpoint = arena_alloc_checkpoint(arena);
    Vector* vector = (Vector*)arena_alloc_push(arena, sizeof(Vector));
    if (!vector) {
        return 0;
    }
    vector->buffer = (u8*)arena_alloc_align(arena);
    if (!vector->buffer) {
        arena_alloc_restore(arena, checkpoint);
        return 0;
    }
    vector->count = 0;
    vector->element_size = element_size;
    vector->arena = arena;
    return vector;
}

void* vector_alloc_push(Vector* vector, void* data) {
    void* allocation = arena_alloc_push_struct_unaligned(vector->arena, data, vector->element_size);
    if (!allocation) {
        return 0;
    }
    vector->count++;
    return allocation;
}

void vector_alloc_pop(Vector* vector) {
    if (!vector || !vector->arena) {
        return;
    }
    if (vector->count > 0) {
        arena_alloc_pop_by(vector->arena, vector->element_size);
        vector->count--;
    }
}

void vector_alloc_pop_zero(Vector* vector) {
    if (!vector || !vector->arena) {
        return;
    }
    if (vector->count > 0) {
        arena_alloc_pop_by_zero(vector->arena, vector->element_size);
        vector->count--;
    }
}

void vector_alloc_clear(Vector* vector) {
    if (!vector) {
        return;
    }
    arena_alloc_reset(vector->arena);
    vector->count = 0;
}

void vector_alloc_clear_zero(Vector* vector) {
    if (!vector) {
        return;
    }
    arena_alloc_reset_zero(vector->arena);
    vector->count = 0;
}

u64 vector_alloc_count(Vector* vector) {
    if (!vector) {
        return 0;
    }
    return vector->count;
}

void* vector_alloc_get(Vector* vector, u64 index) {
    if (!vector) {
        return 0;
    }
    void* data = (void*)(vector->buffer + index * vector->element_size);
    return data;
}

Vector* vector_alloc_copy_to_arena(Arena* arena, Vector* vector) {
    if (!arena || !vector) {
        return 0;
    }
    u64 checkpoint = arena_alloc_checkpoint(arena);
    Vector* new_vector = (Vector*)arena_alloc_push_struct(arena, vector, sizeof(Vector));
    if (!new_vector) {
        return 0;
    }
    void* new_data = arena_alloc_push_struct(arena, vector->buffer, vector->count*vector->element_size);
    if (!new_data) {
        arena_alloc_restore(arena, checkpoint);
        return 0;
    }
    new_vector->buffer = (u8*)new_data;
    new_vector->arena = arena;
    return new_vector;
}
//==============================

