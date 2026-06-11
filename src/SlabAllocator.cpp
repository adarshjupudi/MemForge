#include "../include/SlabAllocator.h"
#include <cstdint>
#include <new>

SlabAllocator::SlabAllocator(void* backing_buffer, size_t capacity, size_t block_size)
    : buffer_start(backing_buffer),
      total_capacity_bytes(capacity),
      object_size(block_size),
      head(nullptr) 
{
          
    // Safety check: Each slot must be at least large enough to hold a 64-bit pointer
    if (object_size < sizeof(Node)) 
    {
        object_size = sizeof(Node);
    }

    // Slice up the buffer and stitch the free-list together
    uintptr_t raw_start = reinterpret_cast<uintptr_t>(buffer_start);
    size_t max_objects = total_capacity_bytes / object_size;

    for (size_t i = 0; i < max_objects; ++i) 
    {
        uintptr_t current_slot_address = raw_start + (i * object_size);
        Node* current_node = reinterpret_cast<Node*>(current_slot_address);
        
        // Push to the front of our head pointer chain
        current_node->next_node = head;
        head = current_node;
    }
}

SlabAllocator::~SlabAllocator() {}

void* SlabAllocator::allocate(size_t size, size_t alignment) 
{
    if (head == nullptr) 
    {
        return nullptr; // Out of memory slots
    }

    // O(1) Pop from the front
    Node* allocated_node = head;
    head = head->next_node;

    // Update Telemetry
    total_allocated_bytes += object_size;
    allocation_count++;

    return reinterpret_cast<void*>(allocated_node);
}

void SlabAllocator::free(void* ptr) 
{
    if (ptr == nullptr) return;

    // O(1) Push to the front
    Node* freed_node = reinterpret_cast<Node*>(ptr);
    freed_node->next_node = head;
    head = freed_node;

    // Update Telemetry
    total_allocated_bytes -= object_size;
    allocation_count--;
}