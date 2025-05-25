#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "memory/allocators.h"
#include "utils/types.h"

/*
// Chained arena
// This is
typedef struct Arena Arena;
struct Arena {
    u8* start;
    u64 top;
    u64 capacity;
    Arena* next;
    bool chain;
};

Arena* arena_create(u64 capacity, bool chain_expand) {
    Arena* arena = (Arena*)malloc(sizeof(Arena));
    if (!arena) {
        printf("Could not allocate space for Arena*\n");
        return(0);
    }
    arena->start = (u8*)malloc(capacity);
    if (!arena->start) {
        free(arena);
        printf("Could not allocate space for the arena's buffer\n");
        return(0);
    }
    memset((void*)arena->start, 0, capacity);
    arena->top = 0;
    arena->capacity = capacity;
    arena->next = 0;
    arena->chain = chain_expand;
    return arena;
}

void* arena_alloc(Arena* arena, u64 size) {
    if (!arena || !arena->start) {
        return(0);
    }
    u64 aligned_size = ALIGN(size);

    // Traverse chain
    Arena* this_arena = arena;
    while(this_arena->next) {
        this_arena = this_arena->next;
    }
    if (this_arena->top + aligned_size > this_arena->capacity) {
        if (!this_arena->chain) {
            printf("Not enough memory left in the allocator!\n");
            return(0);
        }
        Arena* arena_new_next = arena_create(arena->capacity, true);
        if (!arena_new_next) {
            printf("Could not allocate more memory to chain the arena.\n");
            return(0);
        }
        this_arena->next = arena_new_next;
        this_arena = arena_new_next;
    }

    void* allocation = (void*)(this_arena->start + this_arena->top);
    memset(allocation, 0, aligned_size);
    this_arena->top += aligned_size;
    return allocation;
}

void arena_free(Arena* arena) {
    Arena* this_arena = arena;
    while(this_arena) {
        Arena* next_arena = this_arena->next;
        if (this_arena->start) {
            free(this_arena->start);
        }
        free(this_arena);
        this_arena = next_arena;
    }
}
*/

//==============================
// Arena (Linear)
Arena* arena_alloc_create(u64 capacity) {
    Arena* arena = (Arena*) malloc(sizeof(Arena));

    if (!arena) {
        return 0;
    }
    arena->capacity = ALIGN(capacity);
    arena->buffer = (u8*)malloc(arena->capacity);
    if (!arena->buffer) {
        return 0;
    }
    arena->top = 0;
    return arena;
}

Arena* arena_alloc_create_zero(u64 capacity) {
    Arena* arena = arena_alloc_create(capacity);
    if (!arena || !arena->buffer) {
        return 0;
    }
    memset(arena->buffer, 0, arena->capacity);
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
    if (!arena || !arena->buffer) {
        return 0;
    }

    u64 aligned_size = ALIGN(size);

    if (arena->top + aligned_size > arena->capacity) {
        // TODO(alex): Resize once we use mmap/VirtualAlloc by commiting pages
        printf("Arena overflow\n");
        return 0;
    }

    void* allocation = arena->buffer + arena->top;
    arena->top += aligned_size;
    return allocation;
}

void* arena_alloc_push_zero(Arena* arena, u64 size) {
    void* allocation = arena_alloc_push(arena, size);
    if (!allocation) {
        return 0;
    }
    u64 aligned_size = arena->top - ((u64)allocation - (u64)arena->buffer);
    memset(allocation, 0, aligned_size);
    return allocation;
}

void arena_alloc_pop_by(Arena* arena, u64 size) {
    u64 aligned_size = ALIGN(size);
    if(!arena || aligned_size > arena->top) {
        printf("Invalid pop from arena.\n");
        return;
    }
    arena->top -= aligned_size;
}

void arena_alloc_pop_by_zero(Arena* arena, u64 size) {
    if(!arena || size > arena->top) {
        printf("Invalid pop from arena.\n");
        return;
    }
    u64 aligned_size = ALIGN(size);
    arena->top -= aligned_size;
    memset(arena->buffer + arena->top, 0, aligned_size);
}

void arena_alloc_pop_to(Arena* arena, void* addr) {
    if (!arena || !arena->buffer) {
        return;
    }
    if ((u64)addr < (u64)arena->buffer || (u64)addr>=(u64)(arena->buffer+arena->top)) {
        return;
    }

    arena->top = ALIGN((u64)addr - (u64)arena->buffer);
}

void arena_alloc_pop_to_zero(Arena* arena, void* addr) {
    if (!arena || !arena->buffer) {
        return;
    }
    if (addr < (void*)arena->buffer || addr>=(void*)(arena->buffer+arena->top)) {
        return;
    }
    u64 initial_top = arena->top;
    arena->top = ALIGN((u64)addr - (u64)arena->buffer);
    memset(arena->buffer + arena->top, 0, initial_top - arena->top);
}

void arena_alloc_free(Arena* arena) {
    if (arena) {
        if (arena->buffer) {
            free(arena->buffer);
        }
        free(arena);
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
    void* alloc = memcpy(new_alloc, src->buffer, src->top);
    return alloc;
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
            printf("Could not expand memory buffer\n");
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

