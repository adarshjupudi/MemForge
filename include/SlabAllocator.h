#ifndef SLABALLOCATOR_H
#define SLABALLOCATOR_H

#include "IAllocator.h"
#include <cstddef>

class SlabAllocator : public IAllocator 
{
private:
    // The structure representing our empty slots in the free-list
    struct Node 
    {
        Node* next_node;
    };

    void* buffer_start;
    size_t total_capacity_bytes;
    size_t object_size;
    
    // The pointer to the first available empty slot
    Node* head;

public:
    SlabAllocator(void* backing_buffer, size_t capacity, size_t block_size);
    ~SlabAllocator() override;

    void* allocate(size_t size, size_t alignment) override;
    void free(void* ptr) override;
};

#endif // SLABALLOCATOR_H