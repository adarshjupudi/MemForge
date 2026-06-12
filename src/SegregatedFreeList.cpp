#include "../include/SegregatedFreeList.h"
#include <cstdint>
#include <new>

SegregatedFreeList::SegregatedFreeList(void *backing_buffer, size_t capacity)
    : buffer_start(backing_buffer),
      total_capacity_bytes(capacity)
{

    // Initialize all size bins to empty chains
    for (size_t i = 0; i < NUM_BINS; ++i)
    {
        bins[i] = nullptr;
    }

    // Treat the entire backing buffer as one single giant initial free block
    BlockHeader *initial_block = reinterpret_cast<BlockHeader *>(buffer_start);
    initial_block->block_size = total_capacity_bytes - sizeof(BlockHeader);
    initial_block->is_free = true;
    initial_block->next_block = nullptr;

    // Place this giant initial block into the largest bin
    insert_into_bin(initial_block);
}

SegregatedFreeList::~SegregatedFreeList() {}

size_t SegregatedFreeList::get_bin_index(size_t size)
{
    if (size <= 16)
        return 0;
    if (size <= 32)
        return 1;
    if (size <= 64)
        return 2;
    if (size <= 128)
        return 3;
    return 4; // Up to 256 bytes or larger
}

void SegregatedFreeList::insert_into_bin(BlockHeader *block)
{
    size_t bin_idx = get_bin_index(block->block_size);
    block->next_block = bins[bin_idx];
    bins[bin_idx] = block;
}

void SegregatedFreeList::remove_from_bin(BlockHeader *block)
{
    size_t bin_idx = get_bin_index(block->block_size);
    BlockHeader *current = bins[bin_idx];
    BlockHeader *prev = nullptr;

    while (current != nullptr)
    {
        if (current == block)
        {
            if (prev == nullptr)
            {
                bins[bin_idx] = current->next_block;
            }
            else
            {
                prev->next_block = current->next_block;
            }
            return;
        }
        prev = current;
        current = current->next_block;
    }
}

void *SegregatedFreeList::allocate(size_t size, size_t alignment)
{
    size_t target_bin = get_bin_index(size);

    // Search the target bin and higher bins for a free block
    for (size_t i = target_bin; i < NUM_BINS; ++i)
    {
        BlockHeader *block = bins[i];
        while (block != nullptr)
        {
            if (block->is_free && block->block_size >= size)
            {
                // We found a valid block! Pull it out of its free list bin
                remove_from_bin(block);
                block->is_free = false;

                // Splitting Mechanics: Can we cut this block down safely?
                // It's only worth splitting if the remainder can hold another BlockHeader + minimum data
                size_t min_split_size = sizeof(BlockHeader) + 16;
                if (block->block_size >= size + min_split_size)
                {
                    uintptr_t current_addr = reinterpret_cast<uintptr_t>(block);
                    uintptr_t split_addr = current_addr + sizeof(BlockHeader) + size;

                    BlockHeader *remaining_block = reinterpret_cast<BlockHeader *>(split_addr);
                    remaining_block->block_size = block->block_size - size - sizeof(BlockHeader);
                    remaining_block->is_free = true;
                    remaining_block->next_block = nullptr;

                    // Update the size of our allocated block to reflect the cut
                    block->block_size = size;

                    // Re-insert the remaining cut block into the correct smaller bin
                    insert_into_bin(remaining_block);
                }

                total_allocated_bytes += (sizeof(BlockHeader) + block->block_size);
                allocation_count++;

                // Return the memory address that sits directly AFTER the metadata header
                return reinterpret_cast<void *>(reinterpret_cast<uint8_t *>(block) + sizeof(BlockHeader));
            }
            block = block->next_block;
        }
    }
    return nullptr; // Complete allocation failure (No block big enough)
}

void SegregatedFreeList::free(void *ptr)
{
    if (ptr == nullptr)
        return;

    // Shift pointer backward to locate the metadata block header
    BlockHeader *block = reinterpret_cast<BlockHeader *>(reinterpret_cast<uint8_t *>(ptr) - sizeof(BlockHeader));
    block->is_free = true;

    total_allocated_bytes -= (sizeof(BlockHeader) + block->block_size);
    allocation_count--;

    // Re-insert the freed block back into the appropriate bin list
    insert_into_bin(block);

    // Note: Advanced boundary coalescing will hook directly into this phase on Day 8
}