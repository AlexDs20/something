#ifndef _ALLOCATORS_H
#define _ALLOCATORS_H

#include "utils/types.h"

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
void* arena_alloc_push_aligned(Arena* arena, u64 size, u64 alignment);
void* arena_alloc_push_unaligned(Arena* arena, u64 size);
void* arena_alloc_push_zero(Arena* arena, u64 size);
void* arena_alloc_push_zero_aligned(Arena* arena, u64 size, u64 alignment);
void* arena_alloc_push_zero_unaligned(Arena* arena, u64 size);
void* arena_alloc_push_struct(Arena* arena, void* data, u64 size);
void* arena_alloc_push_struct_aligned(Arena* arena, void* data, u64 size, u64 alignment);
void* arena_alloc_push_struct_unaligned(Arena* arena, void* data, u64 size);

void arena_alloc_pop_by(Arena* arena, u64 bytes_to_pop);
void arena_alloc_pop_by_zero(Arena* arena, u64 bytes_to_pop);
void* arena_alloc_pop_to(Arena* arena, void* addr);
void* arena_alloc_pop_to_zero(Arena* arena, void* addr);

void arena_alloc_free(Arena* arena);
void arena_alloc_reset(Arena* arena);
void arena_alloc_reset_zero(Arena* arena);

void* arena_alloc_copy(Arena* dest, Arena* src);
u64 arena_alloc_checkpoint(Arena* arena);
void arena_alloc_restore(Arena* arena, u64 checkpoint);

void arena_debug_map(Arena* arena, u64 width);
void arena_debug_print(Arena* arena);


//==============================
// Vector

typedef struct {
    Arena* arena;
    u64 element_size;
} Vector;

Vector* vector_alloc_create(u64 n, u64 size);
void* vector_alloc_push(Vector* vector, void* data);
void vector_alloc_pop(Vector* vector);
void vector_alloc_pop_zero(Vector* vector);
void vector_alloc_clear(Vector* vector);
void vector_alloc_clear_zero(Vector* vector);
void vector_alloc_free(Vector* vector);
u64 vector_alloc_count(Vector* vector);
void* vector_alloc_get(Vector* vector, u64 index);

#endif  // _ALLOCATORS_H
