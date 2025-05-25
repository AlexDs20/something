#ifndef _ALLOCATORS_H
#define _ALLOCATORS_H

#include "utils/types.h"

#define DEFAULT_ALIGN 4  // Bytes
#define ALIGN(v) ((v)+(DEFAULT_ALIGN-1) & (~(DEFAULT_ALIGN-1)))

typedef struct {
    u8* buffer;
    u64 capacity;
    u64 top;
    u64 committed;
} Arena;


Arena* arena_alloc_create(u64 capacity);
Arena* arena_alloc_create_zero(u64 capacity);
u64 arena_alloc_used(Arena* arena);
u64 arena_alloc_remaining(Arena* arena);
void* arena_alloc_get(Arena* arena, u64 pos);

void* arena_alloc_push(Arena* arena, u64 size);
void* arena_alloc_push_zero(Arena* arena, u64 size);

void arena_alloc_pop_by(Arena* arena, u64 bytes_to_pop);
void arena_alloc_pop_by_zero(Arena* arena, u64 bytes_to_pop);
void arena_alloc_pop_to(Arena* arena, void* loc);
void arena_alloc_pop_to_zero(Arena* arena, void* loc);

void arena_alloc_free(Arena* arena);
void arena_alloc_reset(Arena* arena);
void arena_alloc_reset_zero(Arena* arena);

void* arena_alloc_copy(Arena* dest, Arena* src);

// Resizes the arena's internal buffer to a new capacity. Useful for dynamic arenas.
// If new_capacity < arena->top, reject or truncate.
// If growing, realloc or allocate new memory and copy existing data.
// Update capacity.
//
// bool arena_alloc_resize(Arena* arena, u64 new_capacity);
//
//
// Ensures that the arena has at least the specified capacity. Basically a conditional resize.
// bool arena_alloc_reserve(Arena* arena, u64 min_capacity);
//     If arena->capacity >= min_capacity, do nothing.
//     Else, resize.

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
