#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils/allocators.h"

//==============================
// Arena (Linear)
Arena* arena_allocator_create(unsigned long capacity) {
    Arena* arena = (Arena*) malloc(sizeof(Arena));

    if (!arena) {
        return(0);
    }
    if (capacity < 0) {
        return(0);
    }
    arena->capacity = capacity;
    arena->buffer = (unsigned char*)malloc(arena->capacity);
    if (!arena->buffer) {
        return(0);
    }
    arena->top = 0;
    return arena;
}

void* arena_allocator_alloc(Arena* arena, unsigned long size) {
    if (!arena || !arena->buffer) {
        return(0);
    }

    // unsigned long aligned_size = (size + (DEFAULT_STACK_ALIGN-1)) & (~(DEFAULT_STACK_ALIGN-1));
    unsigned long aligned_size = ALIGN(size);

    if (arena->top + aligned_size > arena->capacity) {
        printf("Arena overflow\n");
        return(0);
    }

    void* allocation = arena->buffer + arena->top;
    arena->top += aligned_size;
    return allocation;
}

// void* arena_allocator_push(Arena* arena, void* src, unsigned long size);

void arena_allocator_pop(Arena* arena, unsigned long size) {
    if(!arena || size > arena->top) {
        printf("Invalid pop from arena.\n");
        return;
    }
    unsigned long aligned_size = ALIGN(size);
    arena->top -= aligned_size;
}

unsigned long arena_allocator_remaining(Arena* arena) {
    if (!arena) {
        return(0);
    }
    return(arena->capacity - arena->top);
}

void arena_allocator_free(Arena* arena) {
    if (arena) {
        if (arena->buffer) {
            free(arena->buffer);
        }
        free(arena);
    }
}

void* arena_allocator_copy(Arena* dest, Arena* src) {
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

void arena_allocator_reset(Arena* arena) {
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
// FreeList

//==============================

//==============================
// Buddy

//==============================
