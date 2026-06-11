#include <iostream>
#include <cstdint>
#include "../include/IAllocator.h"

// Concrete implementation of our Bootcamp mathematical formula
void* align_address(void* current_ptr, size_t alignment) 
{
    uintptr_t raw_address = reinterpret_cast<uintptr_t>(current_ptr);
    uintptr_t aligned_address = (raw_address + (alignment - 1)) & ~(alignment - 1);
    return reinterpret_cast<void*>(aligned_address);
}

int main() 
{
    std::cout << "[MemForge V1.0 initialized]" << std::endl;
    std::cout << "Default System Alignment Requirement: " << DEFAULT_ALIGNMENT << " bytes.\n" << std::endl;

    // Testing our mathematical alignment logic
    void* unaligned_ptr = reinterpret_cast<void*>(11); 
    void* aligned_ptr = align_address(unaligned_ptr, 4);

    std::cout << "Unaligned Address Simulated: " << reinterpret_cast<uintptr_t>(unaligned_ptr) << std::endl;
    std::cout << "Snapped Aligned Address (4-Byte): " << reinterpret_cast<uintptr_t>(aligned_ptr) << std::endl;

    return 0;
}