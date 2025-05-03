#ifndef _ALLOCATORS_H
#define _ALLOCATORS_H

#define DEFAULT_STACK_SIZE 1024
#define DEFAULT_STACK_ALIGN 32
#define ALIGN(v) ((v)+(DEFAULT_STACK_ALIGN-1) & (~(DEFAULT_STACK_ALIGN-1)))

typedef struct {
    unsigned char* buffer;
    unsigned long capacity;
    unsigned long top;
} Arena;

Arena* arena_allocator_create(unsigned long capacity);
void* arena_allocator_alloc(Arena* arena, unsigned long size);
void arena_allocator_pop(Arena* arena, unsigned long size);
void* arena_allocator_copy(Arena* dest, Arena* src);
void arena_allocator_free(Arena* arena);
void arena_allocator_reset(Arena* arena);
unsigned long arena_allocator_remaining(Arena* arena);

#endif  // _ALLOCATORS_H
