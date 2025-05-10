#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils/allocators.h"
#include "utils/defines.h"

typedef struct {
    u8* start;
    u64 top;
    u64 capacity;
} Arena;

Arena* arena_create(u64 capacity) {
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
    return arena;
}

Arena* arena_create_from(void* buffer, u64 capacity) {
    Arena* arena = (Arena*)malloc(sizeof(Arena));
    if (!arena) {
        printf("Could not allocate space for Arena*\n");
        return(0);
    }
    arena->start = (u8*)buffer;
    arena->top = 0;
    arena->capacity = capacity;
    return(arena);
}

void arena_free(Arena* arena) {
    if (arena) {
        if (arena->start) {
            free((void*)arena->start);
        }
        free(arena);
    }
}

void* arena_start(Arena* arena) {
    if (!arena) {
        return(0);
    }
    return (void*)arena->start;
}

u64 arena_top(Arena* arena) {
    if (!arena) {
        return(0);
    }
    return(arena->top);
}

u64 arena_cap(Arena* arena) {
    if (!arena) {
        return(0);
    }
    return(arena->capacity);
}

void* arena_alloc(Arena* arena, u64 size) {
    if (!arena || !arena->start) {
        return(0);
    }
    u64 aligned_size = ALIGN(size);
    if (arena->top + aligned_size > arena->capacity) {
        printf("Not enough memory left in the allocator!\n");
        return(0);
    }

    void* allocation = (void*)(arena->start + arena->top);
    memset(allocation, 0, aligned_size);
    arena->top += aligned_size;
    return allocation;
}

void* arena_alloc_no_zero(Arena* arena, u64 size) {
    if (!arena || !arena->start) {
        return(0);
    }
    u64 aligned_size = ALIGN(size);
    if (arena->top + aligned_size > arena->capacity) {
        printf("Not enough memory left in the allocator!\n");
        return(0);
    }

    void* allocation = arena->start + arena->top;
    arena->top += aligned_size;
    return allocation;
}

void* arena_pop_to(Arena* arena, void* pos) {
    if (!arena || !arena->start) {
        return(0);
    }
    s64 size = (u8*)pos - arena->start;
    if (size<0 || size>arena->capacity) {
        printf("Position larger than available capacity\n");
        return(0);
    }

    memset(pos, 0, size);
    arena->top = size;
    return((void*)(arena->start+arena->top));
}

void* arena_pop_to_no_zero(Arena* arena, void* pos) {
    if (!arena || !arena->start) {
        return(0);
    }
    if (pos>arena->start+arena->capacity) {
        printf("Position larger than available capacity\n");
        return(0);
    }

    arena->top = (u8*)pos-arena->start;
    return((void*)(arena->start+arena->top));
}

void* arena_pop(Arena* arena, u64 size) {
    if (!arena || !arena->start) {
        return(0);
    }
    if (arena->top - size < 0) {
        printf("Trying to pop more than was allocated!\n");
        return(0);
    }
    arena->top -= size;
    memset((void*)(arena->start+arena->top), 0, size);
    return((void*)(arena->start+arena->top));
}

void* arena_pop_no_zero(Arena* arena, u64 size) {
    if (!arena || !arena->start) {
        return(0);
    }
    if (arena->top - size < 0) {
        printf("Trying to pop more than was allocated!\n");
        return(0);
    }
    arena->top -= size;
    return((void*)(arena->start+arena->top));
}

void* arena_clear(Arena* arena) {
    if (!arena || !arena->start) {
        return(0);
    }
    memset((void*)arena->start, 0, arena->top);
    arena->top = 0;
    return(arena->start);
}

void* arena_clear_no_zero(Arena* arena) {
    if (!arena || !arena->start) {
        return(0);
    }
    arena->top = 0;
    return((void*)arena->start);
}

void allocator_main() {
    Arena* arena = arena_create(2*kB);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);

    printf("ALLOC 512\n");
    u8* some_array = (u8*)arena_alloc(arena, 512);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("ALLOC 64\n");
    some_array = (u8*)arena_alloc(arena, 64);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("ALLOC 2048\n");
    some_array = (u8*)arena_alloc(arena, 2048);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("ALLOC NO ZERO 64\n");
    some_array = (u8*)arena_alloc_no_zero(arena, 64);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("POP TO PREVIOUS LOCATION\n");
    some_array = (u8*)arena_pop_to(arena, some_array);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("POP TO PREVIOUS LOCATION-64 NO ZERO\n");
    some_array = (u8*)arena_pop_to_no_zero(arena, (void*)(some_array-64));
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("ARENA POP 64 BYTES\n");
    some_array = (u8*)arena_pop(arena, 64);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("ARENA POP NO ZERO 64 BYTES\n");
    some_array = (u8*)arena_pop(arena, 64);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("ARENA CLEAR\n");
    some_array = (u8*)arena_clear(arena);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("ALLOC 64\n");
    some_array = (u8*)arena_alloc(arena, 64);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("ARENA CLEAR NO ZERO\n");
    some_array = (u8*)arena_clear(arena);
    printf("Arena start: %p\n", arena->start);
    printf("Arena top: %lu\n", arena->top);
    printf("Arena cap: %lu\n", arena->capacity);
    printf("Array location: %p\n", some_array);

    printf("ARENA FREE\n");
    arena_free(arena);
    arena = 0;

    u8 local_buffer[512] = {};
    Arena* arena2 = arena_create_from((void*)local_buffer, 512);
    free(arena2);
    printf("Arena2 freed: %p\n", arena);
}

/*
//==============================
// Arena (Linear)
Arena* arena_alloc_create(unsigned long capacity) {
    Arena* arena = (Arena*) malloc(sizeof(Arena));

    if (!arena) {
        return(0);
    }
    if (capacity < 0) {
        return(0);
    }
    arena->capacity = capacity;
    arena->buffer = (uint8*)malloc(arena->capacity);
    if (!arena->buffer) {
        return(0);
    }
    arena->top = 0;
    return arena;
}

void* arena_alloc_alloc(Arena* arena, unsigned long size) {
    if (!arena || !arena->buffer) {
        return(0);
    }

    // unsigned long aligned_size = (size + (DEFAULT_ALIGN-1)) & (~(DEFAULT_ALIGN-1));
    unsigned long aligned_size = ALIGN(size);

    if (arena->top + aligned_size > arena->capacity) {
        // TODO(alex): Realloc?
        printf("Arena overflow\n");
        return(0);
    }

    void* allocation = arena->buffer + arena->top;
    memset(allocation, 0, aligned_size);
    arena->top += aligned_size;
    return allocation;
}

void arena_alloc_pop(Arena* arena, unsigned long size) {
    if(!arena || size > arena->top) {
        printf("Invalid pop from arena.\n");
        return;
    }
    unsigned long aligned_size = ALIGN(size);
    arena->top -= aligned_size;
}

unsigned long arena_alloc_remaining(Arena* arena) {
    if (!arena) {
        return(0);
    }
    return(arena->capacity - arena->top);
}

void arena_alloc_free(Arena* arena) {
    if (arena) {
        if (arena->buffer) {
            free(arena->buffer);
        }
        free(arena);
    }
}

void* arena_alloc_copy(Arena* dest, Arena* src) {
    if (!src || !dest || (dest == src)) {
        return(0);
    }
    if (dest->capacity < dest->top + src->top) {
        return(0);
    }

    void* alloc = memcpy(dest->buffer + dest->top, src->buffer, src->top);
    dest->top += src->top;
    return alloc;
}

void arena_alloc_reset(Arena* arena) {
    if (arena) {
        arena->top = 0;
    }
}
//==============================

//==============================
// Stack

//==============================

//==============================
// Pool

//==============================

//==============================
// Vector
Vector* vector_alloc_create(uint64 n, uint64 size) {
    Vector* vector = (Vector*)malloc(sizeof(Vector));
    if (!vector) {
        printf("Could not allocate memory for the vector structure.\n");
        return(0);
    }

    vector->buffer = (uint8*)malloc(n*size);
    if(!vector->buffer) {
        printf("Could not allocate memory for the vector buffer.\n");
        free(vector);
        return(0);
    }
    vector->capacity = n*size;
    vector->top = 0;
    vector->element_size = size;
    return vector;
}

void* vector_alloc_push(Vector* vector, void* data) {
    if (!vector) {
        return(0);
    }
    if (vector->top+vector->element_size > vector->capacity) {
        uint8* new_buffer = (uint8*)malloc(2*vector->capacity);
        if (!new_buffer) {
            printf("Could not expand memory buffer\n");
            return(0);
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
    uint8* new_buffer = (uint8*)malloc(vector->top);
    memcpy((void*)new_buffer, (void*)vector->buffer, vector->top);
    free(vector->buffer);
    vector->buffer = new_buffer;
    vector->capacity = vector->top;
}


uint64 vector_alloc_count(Vector* vector) {
    if (vector) {
        return vector->top / vector->element_size;
    }
    return(0);
}

void* vector_alloc_get(Vector* vector, uint64 index) {
    if (index*vector->element_size < vector->top) {
        return vector->buffer + index*vector->element_size;
    }
    return(0);
}

void* vector_alloc_set(Vector* vector, uint64 index, void* data) {
    if (index<vector->top) {
        memcpy((void*)(vector->buffer + index*vector->element_size), data, vector->element_size);
        return vector->buffer + index*vector->element_size;
    }
    return(0);
}
*/


//==============================

