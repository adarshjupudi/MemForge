#include "../include/ArenaAllocator.h"
#include <new>

ArenaAllocator::ArenaAllocator(void* backing_buffer, size_t capacity)
    : buffer_start(backing_buffer), 
      total_capacity_bytes(capacity), 
      current_offset(0) {}

ArenaAllocator::~ArenaAllocator() 
{
    // The Arena does not own the raw buffer memory, so it doesn't free it here.
}

void* ArenaAllocator::allocate(size_t size, size_t alignment) 
{
    // 1. Calculate the raw address where our current offset points
    uintptr_t current_raw_address = reinterpret_cast<uintptr_t>(buffer_start) + current_offset;

    // 2. Apply our Day 1 alignment math formula to find the next valid boundary
    uintptr_t aligned_address = (current_raw_address + (alignment - 1)) & ~(alignment - 1);

    // 3. Convert the aligned address back into an offset relative to our starting point
    size_t new_offset = aligned_address - reinterpret_cast<uintptr_t>(buffer_start) + size;

    // 4. Boundary Check: Ensure we haven't run out of physical space in our Arena
    if (new_offset > total_capacity_bytes) 
    {
        return nullptr; // Out of memory!
    }

    // 5. Update our trackable state metrics (Telemetry)
    current_offset = new_offset;
    total_allocated_bytes = current_offset;
    allocation_count++;

    // 6. Return the aligned address as a raw void pointer to the application
    return reinterpret_cast<void*>(aligned_address);
}

void ArenaAllocator::free(void* ptr) 
{
    // This is an intentional NO-OP (No Operation).
    // Individual free operations are illegal in a Bump Allocator to maintain O(1) speeds.
}

void ArenaAllocator::reset() 
{
    // Instantly reclaims all memory by sliding our tracking flag back to the start
    current_offset = 0;
    total_allocated_bytes = 0;
    allocation_count = 0;
}