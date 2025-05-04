#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils/allocators.h"

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
        printf("Could not allocate memory for the vector buffer");
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


/*
uint64 vector_alloc_size(Vector* vector) {
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
