#include <sys/mman.h>
#include <stddef.h>
#include <unistd.h>

typedef struct {
    void* ptr;
    size_t size;
} MemoryBlock;

MemoryBlock os_memory_alloc(u64 size) {
    void* pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    MemoryBlock block = {0};
    if (pa == MAP_FAILED) {
        return block;
    }
    block = { .ptr = pa, .size = size };
    return block;
}

bool os_memory_free(void* addr, u64 size) {
   return munmap(addr, size) == 0;        // munmap => 0 = success , -1 = error
}

MemoryBlock os_memory_reserve(u64 size) {
    void* pa = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    MemoryBlock block = {0};
    if (pa == MAP_FAILED) {
        return block;
    }
    block = { .ptr = pa, .size = size };
    return block;
}

bool os_memory_commit(void* addr, u64 size) {
    // Force align to page size
    // long page_size = sysconf(_SC_PAGESIZE);
    // void* aligned_addr = (void*)((u64)addr & ~(page_size - 1));
    return mprotect(addr, size, PROT_READ | PROT_WRITE) == 0;       // mprotect => 0 = success, -1 = error
}

bool os_memory_decommit(void* addr, u64 size) {
    if (mprotect(addr, size, PROT_NONE) != 0) {
        return false;
    }
    // Could also suggest os we don't need
    madvise(addr, size, MADV_DONTNEED);
    return true;
}
