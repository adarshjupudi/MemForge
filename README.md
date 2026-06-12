# MemForge: High-Performance Hybrid Memory Management Engine

MemForge is a custom memory management engine written in **C++17** that bypasses the standard heap allocator during runtime and uses specialized allocation strategies for predictable, low-latency memory operations.

It combines multiple allocator designs under a unified memory manager:

* Slab Allocator (fixed-size allocations)
* Segregated Free List (variable-size allocations)
* Arena Allocator (transient allocations)
* Central Memory Manager (allocation routing & telemetry)

---

##  Benchmark Results

Stress Test: **50,000 allocations and deallocations** of a 144-byte `GameEntity`.

```text
========================================
           PERFORMANCE SUMMARY
========================================

Standard Heap Time : 15757 us
MemForge Time      : 3017 us

Result             : 5.22x Faster
```

### Telemetry

```text
Active Allocations : 0
Peak Memory Use    : 7200000 bytes
STATUS             : Clean. Zero leaks detected.
```

---

##  Architecture

```text
+----------------------------------------------------+
|              Central Memory Manager                |
+----------------------------------------------------+
          |             |              |
          v             v              v

   Slab Allocator   Free List      Arena Allocator
                     Allocator
```

### Slab Allocator

* O(1) allocation
* O(1) deallocation
* Fixed-size object pools
* Eliminates external fragmentation

### Segregated Free List

* Power-of-two size bins
* Dynamic block splitting
* Block coalescing
* Variable-size allocation support

### Arena Allocator

* Pointer-bump allocation
* Extremely low overhead
* Bulk memory reclamation through `reset()`

### Central Memory Manager

* Owns the memory pool
* Routes allocations/deallocations
* Maintains telemetry
* O(1) allocator ownership detection via pointer ranges

---

##  Build & Run

```bash
g++ -O3 -std=c++17 \
src/main.cpp \
src/ArenaAllocator.cpp \
src/SlabAllocator.cpp \
src/SegregatedFreeList.cpp \
src/CentralMemoryManager.cpp \
-o MemForgeEngine

./MemForgeEngine
```

---

## 💡 Engineering Trade-Offs

### Advantages

* Low allocation latency
* Deterministic allocation behavior
* Reduced fragmentation
* Cache-friendly memory reuse
* Built-in telemetry and leak tracking

### Limitations

* Fixed memory budget
* No automatic heap expansion
* Optimized for predictable allocation patterns
* Not intended as a general-purpose heap replacement

---

##  Ideal Use Cases

* Game Engines
* Entity Component Systems (ECS)
* Real-Time Simulations
* High-Frequency Trading Systems
* Network Packet Processing
* Performance-Critical Backend Services

---

## License

Educational and research purposes.
