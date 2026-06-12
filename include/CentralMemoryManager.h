#ifndef CENTRALMEMORYMANAGER_H
#define CENTRALMEMORYMANAGER_H

#include "IAllocator.h"
#include "ArenaAllocator.h"
#include "SlabAllocator.h"
#include "SegregatedFreeList.h"
#include <cstddef>

class CentralMemoryManager : public IAllocator
{
private:
    // Backing raw byte spaces for our subsystems
    void *master_buffer;
    size_t master_capacity;

    // Sub-allocators managed internally
    SlabAllocator *slab_16;
    SlabAllocator *slab_64;
    SegregatedFreeList *general_freelist;
    ArenaAllocator *transient_arena;

public:
    CentralMemoryManager(void *raw_os_buffer, size_t total_size);
    ~CentralMemoryManager() override;

    void *allocate(size_t size, size_t alignment) override;
    void *allocate_transient(size_t size, size_t alignment); // Routes directly to Arena
    void free(void *ptr) override;

    void reset_transient_pool(); // Flushes the Arena pool instantly
};

#endif // CENTRALMEMORYMANAGER_H