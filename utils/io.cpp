#include "io.h"

#include <sys/mman.h>       // mmap
#include <sys/stat.h>       // stat
#include <stdio.h>          // perror
#include <fcntl.h>          // open, O_RDONLY
#include <stdlib.h>         // exit, ...
#include <unistd.h>         // close, write


typedef struct {
    char* data;
    size_t size;
} string;

void read_complete_file(char* filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
      perror("open");
      exit(1);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
      perror("fstat");
      exit(2);
    }

    char* data = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if(data == MAP_FAILED) {
      close(fd);
      perror("mmap");
      exit(3);
    }

    // Print file contents
    write(1, data, sb.st_size);

    munmap(data, sb.st_size);
    close(fd);
}

void anonymous_memory_allocation() {
    const int NUM_INTS = 100;
    int *data;

    data = (int*)mmap(NULL, NUM_INTS * sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if(data == MAP_FAILED) {
     perror("mmap");
     exit(1);
    }

    // Use the memory
    for(int i = 0; i < NUM_INTS; i++) {
        data[i] = i;
    }

    // Clean up
    munmap(data, NUM_INTS * sizeof(int));
}


constexpr size_t MAX_VECTOR_SIZE = 1ULL << 30; // 1 GB reserved
constexpr size_t PAGE_SIZE = 4096;

void* reserve_space(size_t size) {
    void* ptr = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return nullptr;
    }
    return ptr;
}

bool commit_page(void* addr) {
    if (mprotect(addr, PAGE_SIZE, PROT_READ | PROT_WRITE) != 0) {
        perror("mprotect");
        return false;
    }
    return true;
}


#include <string.h>
void map_pages() {
    void* vec_space = reserve_space(MAX_VECTOR_SIZE);
    if (!vec_space) return;

    // Example: Grow vector up to 10 pages
    for (size_t i = 0; i < 10; ++i) {
        void* page_addr = static_cast<char*>(vec_space) + i * PAGE_SIZE;
        if (!commit_page(page_addr)) return;

        // Now it's safe to write
        memset(page_addr, 0, PAGE_SIZE);
        printf("Committed and zeroed page %d\n", i);
    }

    munmap(vec_space, MAX_VECTOR_SIZE);
}

void main_io() {
    char file[] = "assets/backpack/backpack.obj";
    // read_complete_file(file);
    // anonymous_memory_allocation();
    map_pages();
}
