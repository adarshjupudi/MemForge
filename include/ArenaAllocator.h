#ifndef ARENAALLOCATOR_H
#define ARENAALLOCATOR_H

#include "IAllocator.h"
#include <cstddef>
#include <cstdint>

class ArenaAllocator : public IAllocator 
{
    private:
    void* buffer_start;
    size_t total_capacity_bytes;
    size_t current_offset;

    public:
    ArenaAllocator(void* backing_buffer, size_t capacity);
    ~ArenaAllocator() override;

    void* allocate(size_t size, size_t alignment) override;
    void free(void* ptr) override;
    
    void reset();
};

#endif // ARENAALLOCATOR_H