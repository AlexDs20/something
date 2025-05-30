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
/*
typedef struct {
    Arena* arena;
    u64 element_size;
    u64 element_alignment;
    u64 arena_checkpoint;
} Vector2;

Vector2* vector2_alloc_create(Arena* arena, u64 element_size, u64 element_alignement);
void* vector2_alloc_push(Vector2* vector, void* data);
void vector2_alloc_pop(Vector2* vector);
void* vector2_alloc_get(Vector2* vector, u64 index);
u64 vector2_alloc_count(Vector2* vector);
void vector2_alloc_clear(Vector2* vector);

Vector2* vector2_alloc_create(Arena* arena, u64 element_size, u64 element_alignement) {
    out->arena_checkpoint = arena_alloc_checkpoint(arena);
    Vector2* out = (Vector2*)arena_alloc_push(arena, sizeof(Vector2));
    out->element_size = element_size;
    out->element_alignment = element_alignement;
    return out;
}

void* vector2_alloc_push(Vector2* vector, void* data) {
    return arena_alloc_push_struct_aligned(vector->arena, data, vector->element_size, vector->element_alignment);
}

void* vector2_alloc_pop(Vector2* vector, void* data) {
    return arena_alloc_pop_by(vector->arena, vector->element_size);
}

void* vector2_alloc_get(Vector2* vector, u64 pos) {
    return arena_alloc_get(vector->arena, pos * vector->element_alignment);
}

u64 vector2_alloc_count(Vector2* vector) {
    return arena_alloc_used(vector->arena) / vector->element_alignment;
}

void vector2_alloc_clear(Vector2* vector) {
    arena_alloc_restore(vector->arena, vector->arena_checkpoint);
}
*/
#endif  // _ALLOCATORS_H
