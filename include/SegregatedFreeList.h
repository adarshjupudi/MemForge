#ifndef SEGREGATEDFREELIST_H
#define SEGREGATEDFREELIST_H

#include "IAllocator.h"
#include <cstddef>

class SegregatedFreeList : public IAllocator
{
private:
    struct BlockHeader
    {
        size_t block_size;
        bool is_free;
        BlockHeader *next_block;
    };

    static const size_t NUM_BINS = 5;
    BlockHeader *bins[NUM_BINS];

    void *buffer_start;
    size_t total_capacity_bytes;

    size_t get_bin_index(size_t size);
    void insert_into_bin(BlockHeader *block);
    void remove_from_bin(BlockHeader *block);

    public:
    SegregatedFreeList(void *backing_buffer, size_t capacity);
    ~SegregatedFreeList() override;

    void *allocate(size_t size, size_t alignment) override;
    void free(void *ptr) override;
};

#endif // SEGREGATEDFREELIST_H