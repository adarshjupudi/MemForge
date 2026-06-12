#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <cstddef>
#include <iostream>

struct TelemetryReport
{
    size_t current_bytes_in_use;
    size_t peak_bytes_in_use; // High-water mark
    size_t active_allocations;
    size_t leaked_blocks_count;
};

class TelemetrySuite
{
private:
    size_t high_water_mark;

public:
    TelemetrySuite() : high_water_mark(0) {}

    void update_high_water_mark(size_t current_usage)
    {
        if (current_usage > high_water_mark)
        {
            high_water_mark = current_usage;
        }
    }

    void generate_snapshot(size_t current_bytes, size_t alloc_count)
    {
        update_high_water_mark(current_bytes);

        std::cout << "\n========================================\n";
        std::cout << "       MEMFORGE SYSTEM TELEMETRY        \n";
        std::cout << "========================================\n";
        std::cout << " Active Allocations : " << alloc_count << "\n";
        std::cout << " Current Memory Use : " << current_bytes << " bytes\n";
        std::cout << " Peak Memory Use    : " << high_water_mark << " bytes (High-Water Mark)\n";

        if (current_bytes > 0 && alloc_count > 0)
        {
            std::cout << " LEAK WARNING       : " << alloc_count << " block(s) still active in memory!\n";
        }
        else
        {
            std::cout << " STATUS             : Clean. Zero leaks detected.\n";
        }
        std::cout << "========================================\n\n";
    }
};

#endif // TELEMETRY_H