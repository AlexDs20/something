#ifndef _ALLOCATORS_H
#define _ALLOCATORS_H

#include "utils/defines.h"

#define DEFAULT_ALIGN 32
#define ALIGN(v) ((v)+(DEFAULT_ALIGN-1) & (~(DEFAULT_ALIGN-1)))

typedef struct {
    uint8* buffer;
    uint64 capacity;
    uint64 top;
} Arena;

typedef struct {

} Stack;

typedef struct {

} Pool;

Arena* arena_alloc_create(uint64 capacity);
void* arena_alloc_alloc(Arena* arena, uint64 size);
void arena_alloc_pop(Arena* arena, uint64 size);
void* arena_alloc_copy(Arena* dest, Arena* src);
void arena_alloc_free(Arena* arena);
void arena_alloc_reset(Arena* arena);
uint64 arena_alloc_remaining(Arena* arena);

#endif  // _ALLOCATORS_H
