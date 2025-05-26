#include <sys/mman.h>
#include <unistd.h>

#include <stdio.h>

#include "platform/memory.h"

#define ALIGN_TO(v, a) (((v)+((a)-1)) & (~((a)-1)))

MemoryBlock os_memory_alloc(u64 size) {
    void* pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pa == MAP_FAILED) {
        MemoryBlock block = {0};
        return block;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    u64 aligned_size = ALIGN_TO(size, page_size);
    MemoryBlock block = { .ptr = pa, .size = aligned_size, .committed = aligned_size };
    return block;
}

bool os_memory_free(void* addr, u64 size) {
   return munmap(addr, size) == 0;        // munmap => 0 = success , -1 = error
}

MemoryBlock os_memory_reserve(u64 size) {
    void* pa = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pa == MAP_FAILED) {
        MemoryBlock block = {0};
        return block;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    MemoryBlock block = { .ptr = pa, .size = ALIGN_TO(size, page_size), .committed = 0 };
    return block;
}

MemoryBlock os_memory_commit(void* addr, u64 size) {
    // Force align to page size
    long page_size = sysconf(_SC_PAGESIZE);
    // void* aligned_addr = (void*)((u64)addr & ~(page_size - 1));          // Aligns down to the beginning of the page
    bool success = mprotect(addr, size, PROT_READ | PROT_WRITE) == 0;       // mprotect => 0 = success, -1 = error
    if (!success) {
        MemoryBlock block = {0};
        return block;
    }
    MemoryBlock block = { .ptr = addr, .size=ALIGN_TO(size, page_size), .committed=ALIGN_TO(size, page_size) };
    return block;
}

bool os_memory_decommit(void* addr, u64 size) {
    if (mprotect(addr, size, PROT_NONE) != 0) {
        return false;
    }
    // Could also suggest os we don't need
    madvise(addr, size, MADV_DONTNEED);
    return true;
}

u64 os_memory_page_size() {
    return sysconf(_SC_PAGESIZE);
}
