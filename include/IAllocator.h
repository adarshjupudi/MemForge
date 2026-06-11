#ifndef IALLOCATOR_H
#define IALLOCATOR_H

#include <cstddef> // Required for size_t

class IAllocator {
protected:
    size_t total_allocated_bytes;
    size_t allocation_count;

public:
    IAllocator() : total_allocated_bytes(0), allocation_count(0) {}
    virtual ~IAllocator() {}

    // Pure virtual functions that every child allocator MUST implement
    virtual void* allocate(size_t size, size_t alignment) = 0;
    virtual void free(void* ptr) = 0;

    // Day 9 Telemetry hooks: Read-only accessors for memory tracking
    size_t getTotalAllocatedBytes() const { return total_allocated_bytes; }
    size_t getAllocationCount() const { return allocation_count; }
};

#define DEFAULT_ALIGNMENT (alignof(std::max_align_t))

#endif // IALLOCATOR_H