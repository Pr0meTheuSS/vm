#include "memory_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

static void* shared_memory = NULL;
static size_t current_offset = 0;

void init_memory() {
    shared_memory = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
}

void cleanup_memory() {
    if (munmap(shared_memory, MEMORY_SIZE) == -1) {
        perror("munmap failed");
    }
}

void* get_shared_memory() {
    return shared_memory;
}

size_t allocate_offset(size_t size) {
    size_t offset = current_offset;
    current_offset += size;
    if (current_offset > MEMORY_SIZE) {
        fprintf(stderr, "Memory overflow: not enough space!\n");
        exit(1);
    }
    return offset;
}
