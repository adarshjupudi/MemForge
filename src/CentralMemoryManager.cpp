#include "../include/CentralMemoryManager.h"
#include <cstdint>
#include <new>

CentralMemoryManager::CentralMemoryManager(void *raw_os_buffer, size_t total_size)
    : master_buffer(raw_os_buffer),
      master_capacity(total_size)
{

    // Divide our total budget into 4 distinct equal structural chunks
    size_t quarter_size = total_size / 4;
    uintptr_t base_addr = reinterpret_cast<uintptr_t>(master_buffer);

    // Subdivide the memory map space manually using pointer offsets
    void *chunk1 = reinterpret_cast<void *>(base_addr);
    void *chunk2 = reinterpret_cast<void *>(base_addr + quarter_size);
    void *chunk3 = reinterpret_cast<void *>(base_addr + (quarter_size * 2));
    void *chunk4 = reinterpret_cast<void *>(base_addr + (quarter_size * 3));

    // Construct the specialized subsystems using our placement new pattern
    slab_16 = new (chunk1) SlabAllocator(reinterpret_cast<uint8_t *>(chunk1) + sizeof(SlabAllocator), quarter_size - sizeof(SlabAllocator), 16);
    slab_64 = new (chunk2) SlabAllocator(reinterpret_cast<uint8_t *>(chunk2) + sizeof(SlabAllocator), quarter_size - sizeof(SlabAllocator), 64);
    general_freelist = new (chunk3) SegregatedFreeList(reinterpret_cast<uint8_t *>(chunk3) + sizeof(SegregatedFreeList), quarter_size - sizeof(SegregatedFreeList));
    transient_arena = new (chunk4) ArenaAllocator(reinterpret_cast<uint8_t *>(chunk4) + sizeof(ArenaAllocator), quarter_size - sizeof(ArenaAllocator));
}

CentralMemoryManager::~CentralMemoryManager()
{
    // Explicitly call destructors for subsystems since they were built via placement new
    slab_16->~SlabAllocator();
    slab_64->~SlabAllocator();
    general_freelist->~SegregatedFreeList();
    transient_arena->~ArenaAllocator();
}

void *CentralMemoryManager::allocate(size_t size, size_t alignment)
{
    allocation_count++;

    // Routing Strategy Rule 1: Small objects go to the dedicated O(1) Slab pools
    if (size <= 16)
    {
        total_allocated_bytes += 16;
        return slab_16->allocate(size, alignment);
    }
    if (size <= 64)
    {
        total_allocated_bytes += 64;
        return slab_64->allocate(size, alignment);
    }

    // Routing Strategy Rule 2: Varied/Large requests fall back to the Segregated Free-List
    total_allocated_bytes += size;
    return general_freelist->allocate(size, alignment);
}

void *CentralMemoryManager::allocate_transient(size_t size, size_t alignment)
{
    allocation_count++;
    total_allocated_bytes += size;
    // Bypasses pools completely for immediate frame calculations
    return transient_arena->allocate(size, alignment);
}

void CentralMemoryManager::free(void *ptr)
{
    if (ptr == nullptr)
        return;

    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t base_addr = reinterpret_cast<uintptr_t>(master_buffer);
    size_t quarter_size = master_capacity / 4;

    // Check where the pointer's memory location sits inside our subdivided regions
    if (addr >= base_addr && addr < base_addr + quarter_size)
    {
        slab_16->free(ptr);
        total_allocated_bytes -= 16;
    }
    else if (addr >= base_addr + quarter_size && addr < base_addr + (quarter_size * 2))
    {
        slab_64->free(ptr);
        total_allocated_bytes -= 64;
    }
    else if (addr >= base_addr + (quarter_size * 2) && addr < base_addr + (quarter_size * 3))
    {
        general_freelist->free(ptr);
        total_allocated_bytes -= 0; // The internal header tracking accounts for actual free sizes
    }
    // Arena pointers (quarter 4) ignore individual freeing entirely until reset_transient_pool()

    allocation_count--;
}

void CentralMemoryManager::reset_transient_pool()
{
    transient_arena->reset();
}