#ifndef _ALLOCATORS_H
#define _ALLOCATORS_H

#include "utils/defines.h"

#define DEFAULT_ALIGN 4
#define ALIGN(v) ((v)+(DEFAULT_ALIGN-1) & (~(DEFAULT_ALIGN-1)))

typedef struct {
    u8* buffer;
    u64 capacity;
    u64 top;
} Arena;

typedef struct {

} Stack;

typedef struct {

} Pool;

Arena* arena_alloc_create(u64 capacity);
void* arena_alloc_alloc(Arena* arena, u64 size);
void arena_alloc_pop(Arena* arena, u64 size);
void* arena_alloc_copy(Arena* dest, Arena* src);
void arena_alloc_free(Arena* arena);
void arena_alloc_reset(Arena* arena);
u64 arena_alloc_remaining(Arena* arena);


//==============================
// Vector

typedef struct {
    u8* buffer;
    u64 capacity;
    u64 top;
    u64 element_size;
} Vector;

Vector* vector_alloc_create(u64 n, u64 size);
void* vector_alloc_push(Vector* vector, void* data);
void vector_alloc_pop(Vector* vector);
void vector_alloc_free(Vector* vector);
void vector_alloc_trim(Vector* vector);
u64 vector_alloc_count(Vector* vector);
void* vector_alloc_get(Vector* vector, u64 index);

#endif  // _ALLOCATORS_H
