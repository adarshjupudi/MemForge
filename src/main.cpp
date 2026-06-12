#include <iostream>
#include <cstdint>
#include <chrono>
#include <vector>
#include <new>

#include "../include/CentralMemoryManager.h"
#include "../include/Telemetry.h"

// Complex struct to simulate data objects
struct GameEntity
{
    uint64_t id;
    double matrix[4][4];
    uint32_t health;
};

const size_t SIMULATION_SCALE = 50000;         // 50,000 operations
const size_t MEMORY_BUDGET = 32 * 1024 * 1024; // 32MB pre-allocated pool

int main()
{
    std::cout << "========================================\n";
    std::cout << "      MEMFORGE MICRO-BENCHMARK SUITE    \n";
    std::cout << "========================================\n\n";

    // Allocate the massive raw backing arena directly from the system stack memory
    std::vector<uint8_t> system_ram(MEMORY_BUDGET);

    // Initialize our master engine and telemetry tracker
    CentralMemoryManager manager(system_ram.data(), MEMORY_BUDGET);
    TelemetrySuite tracker;

    // Keep track of our raw pointers for cleanup
    void *pointers[SIMULATION_SCALE];

    // ========================================
    // BENCHMARK 1: STANDARD C++ HEAP (new / delete)
    // ========================================
    std::cout << "[Running Benchmark 1/2: Standard C++ Heap...]\n";
    auto start_standard = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < SIMULATION_SCALE; ++i)
    {
        pointers[i] = ::operator new(sizeof(GameEntity));
    }
    for (size_t i = 0; i < SIMULATION_SCALE; ++i)
    {
        ::operator delete(pointers[i]);
    }

    auto end_standard = std::chrono::high_resolution_clock::now();
    auto duration_standard = std::chrono::duration_cast<std::chrono::microseconds>(end_standard - start_standard).count();

    // ========================================
    // BENCHMARK 2: CUSTOM MEMFORGE HYBRID MANAGER
    // ========================================
    std::cout << "[Running Benchmark 2/2: MemForge Engine...]\n";
    auto start_custom = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < SIMULATION_SCALE; ++i)
    {
        pointers[i] = manager.allocate(sizeof(GameEntity), alignof(GameEntity));
    }

    // Mid-simulation telemetry capture (High-Water Mark profiling)
    tracker.update_high_water_mark(manager.getTotalAllocatedBytes());

    for (size_t i = 0; i < SIMULATION_SCALE; ++i)
    {
        manager.free(pointers[i]);
    }

    auto end_custom = std::chrono::high_resolution_clock::now();
    auto duration_custom = std::chrono::duration_cast<std::chrono::microseconds>(end_custom - start_custom).count();

    // ========================================
    // DISPLAY RESULTS & TELEMETRY SNAPSHOTS
    // ========================================
    std::cout << "\n========================================\n";
    std::cout << "           PERFORMANCE SUMMARY          \n";
    std::cout << "========================================\n";
    std::cout << " Standard Heap Time : " << duration_standard << " us\n";
    std::cout << " MemForge Engine Time: " << duration_custom << " us\n";

    if (duration_custom < duration_standard)
    {
        double speedup = static_cast<double>(duration_standard) / duration_custom;
        std::cout << " Result              : MemForge is " << speedup << "x FASTER!\n";
    }
    std::cout << "========================================\n";

    // Generate telemetry profile to prove zero memory leaks remain
    tracker.generate_snapshot(manager.getTotalAllocatedBytes(), manager.getAllocationCount());

    return 0;
}