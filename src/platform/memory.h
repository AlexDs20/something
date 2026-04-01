#ifndef PLATFORM_MEMORY_H
#define PLATFORM_MEMORY_H
#include <stddef.h>
#include <unistd.h>
#include "base/base.h"

typedef struct {
    void* ptr;
    size_t size;
} MemoryBlock;

MemoryBlock os_memory_alloc(u64 size);
bool os_memory_free(void* addr, u64 size);
MemoryBlock os_memory_reserve(u64 size);
MemoryBlock os_memory_commit(void* addr, u64 size);
bool os_memory_decommit(void* addr, u64 size);
u64 os_memory_page_size();

#endif  // PLATFORM_MEMORY_H
