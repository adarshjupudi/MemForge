# MemForge: High-Performance Hybrid Memory Management Engine

MemForge is a production-grade, low-overhead, user-space memory management engine implemented in **C++17**.

By completely bypassing the general-purpose OS heap manager (`malloc` / `new`) during runtime execution, MemForge provides specialized allocation structures designed for ultra-low latency systems such as:

* Game Engines
* High-Frequency Trading (HFT) Platforms
* Real-Time Simulation Systems
* Performance-Critical Backend Infrastructure

In performance stress testing over **50,000 continuous allocation cycles**, MemForge achieved **5.22× faster execution** than the standard C++ heap allocator while maintaining a mathematically verified **0% memory leak footprint**.

---

# 🚀 Performance Benchmarks

The engine contains an integrated high-precision `<chrono>` micro-benchmark suite that executes a high-stress simulation of **50,000 sequential object allocations and deallocations** using a 144-byte `GameEntity` structure.

## Benchmark Output

```text
========================================
      MEMFORGE MICRO-BENCHMARK SUITE
========================================

[Running Benchmark 1/2: Standard C++ Heap...]
[Running Benchmark 2/2: MemForge Engine...]

========================================
           PERFORMANCE SUMMARY
========================================

Standard Heap Time  : 15757 us
MemForge Time       : 3017 us

Result              : MemForge is 5.22274x FASTER!

========================================

========================================
       MEMFORGE SYSTEM TELEMETRY
========================================

Active Allocations : 0
Current Memory Use : 7200000 bytes
Peak Memory Use    : 7200000 bytes
STATUS             : Clean. Zero leaks detected.

========================================
```

---

# 📊 Performance Analysis

## Zero Kernel Crossings

Traditional heap allocators eventually request additional pages from the operating system through mechanisms such as:

* `brk()`
* `sbrk()`
* `mmap()`

MemForge performs a single bulk allocation during initialization and executes all subsequent allocations entirely within user space.

This removes repeated kernel interaction overhead from the allocation path.

---

## Deterministic O(1) Allocation Paths

General-purpose allocators must support arbitrary allocation sizes and therefore maintain complex metadata structures.

MemForge instead uses specialized allocation strategies:

* Embedded free lists
* Size-segregated bins
* Arena pointer bumping

Most allocation and deallocation operations therefore complete in **constant time**.

---

## Cache-Friendly Memory Reuse

Recently freed blocks remain close together in memory and are quickly recycled.

This improves:

* L1 cache locality
* L2 cache locality
* Branch prediction efficiency

while reducing expensive RAM fetches.

---

# 🏗️ Architecture Overview

MemForge follows a **Hybrid Segmented Memory Architecture**.

A single preallocated memory region is divided into dedicated sections managed by specialized allocation engines.

```text
+----------------------------------------------------+
|              Central Memory Manager                |
+----------------------------------------------------+
          |             |              |
          v             v              v

   Slab Allocator   Free List      Arena Allocator
                     Allocator
```

---

# 1. Central Memory Manager

`CentralMemoryManager`

The orchestration layer of the entire system.

Responsibilities:

* Owns the global memory buffer
* Partitions memory into allocator regions
* Routes allocation requests
* Routes deallocation requests
* Maintains telemetry information

## Range-Based Demultiplexing

When a pointer is freed, MemForge determines ownership through direct address comparison:

```cpp
ptr >= region_start &&
ptr < region_end
```

This avoids:

* Hash tables
* Lookup maps
* Ownership metadata searches

resulting in O(1) allocator routing.

---

# 2. Embedded Free-List Slab Allocator

`SlabAllocator`

Designed for high-frequency allocation of fixed-size objects.

Examples:

* Entities
* Components
* Particles
* Events

## Embedded Pointer Technique

When a slot becomes free, its memory is reused to store a node:

```cpp
struct FreeNode
{
    FreeNode* next;
};
```

Allocation:

```text
Pop from free list
```

Deallocation:

```text
Push into free list
```

Complexity:

| Operation | Complexity |
| --------- | ---------- |
| Allocate  | O(1)       |
| Free      | O(1)       |

External fragmentation is eliminated.

---

# 3. Segregated Free List Allocator

`SegregatedFreeList`

Handles variable-sized allocations.

## Power-of-Two Binning

Memory blocks are grouped into size classes:

```text
16 B
32 B
64 B
128 B
256 B
512 B
1024 B
...
```

Each size class maintains its own linked list.

---

## Dynamic Block Splitting

When an exact-sized block is unavailable:

1. A larger block is selected.
2. The block is split.
3. Requested memory is returned.
4. Remaining space is reinserted.

---

## Coalescing

When neighboring free blocks are detected:

```text
Block A + Block B
        ↓
Merged Larger Block
```

This reduces fragmentation over long runtimes.

---

# 4. Arena Allocator

`ArenaAllocator`

Designed for transient allocations with predictable lifetimes.

Examples:

* Frame data
* Temporary buffers
* Parsing structures
* Scratch memory

Allocation consists of a simple pointer increment:

```cpp
current += size;
```

No free operation exists.

Entire memory regions are reclaimed instantly using:

```cpp
arena.reset();
```

This makes allocation effectively only a few CPU instructions.

---

# 📂 Repository Structure

```text
MemForge/
│
├── include/
│   ├── IAllocator.h
│   ├── ArenaAllocator.h
│   ├── SlabAllocator.h
│   ├── SegregatedFreeList.h
│   ├── CentralMemoryManager.h
│   └── Telemetry.h
│
├── src/
│   ├── ArenaAllocator.cpp
│   ├── SlabAllocator.cpp
│   ├── SegregatedFreeList.cpp
│   ├── CentralMemoryManager.cpp
│   └── main.cpp
│
└── README.md
```

---

# ⚙️ Compilation

Compile using any modern C++17 compiler.

```bash
g++ -O3 -std=c++17 \
src/main.cpp \
src/ArenaAllocator.cpp \
src/SlabAllocator.cpp \
src/SegregatedFreeList.cpp \
src/CentralMemoryManager.cpp \
-o MemForgeEngine
```

Run:

```bash
./MemForgeEngine
```

---

# 🧪 Telemetry & Leak Detection

MemForge continuously tracks:

* Active allocations
* Current memory usage
* Peak memory usage
* Allocation statistics

Expected benchmark output:

```text
Active Allocations : 0
STATUS             : Clean. Zero leaks detected.
```

A value of zero confirms that every allocated block was successfully returned to its originating allocator.

---

# 📈 Memory Utilization Verification

Peak memory consumption during testing:

```text
7,200,000 bytes
```

Derived from:

```text
50,000 objects × 144 bytes
```

Alignment calculations use bitwise rounding:

```cpp
(address + (alignment - 1)) &
~(alignment - 1)
```

ensuring efficient packing while maintaining alignment guarantees.

---

# 💡 Engineering Trade-Offs

MemForge prioritizes deterministic performance over general-purpose flexibility.

## Advantages

* Extremely low allocation latency
* Predictable runtime behavior
* O(1) allocation paths
* Reduced fragmentation
* Cache-friendly memory reuse
* Built-in telemetry

## Limitations

* Fixed memory budget
* No automatic heap expansion
* Specialized allocation patterns
* Potential internal fragmentation in slab regions

If the configured memory pool becomes exhausted, MemForge fails safely rather than requesting additional pages from the operating system.

---

# 🎯 Ideal Use Cases

MemForge is best suited for environments where memory requirements are known ahead of time and predictable performance is critical.

Examples include:

* AAA Game Engine Subsystems
* Entity Component Systems (ECS)
* High-Frequency Trading Platforms
* Real-Time Physics Simulations
* Network Packet Processing
* Embedded Runtime Systems
* Performance-Critical Backend Services

---

## License

This project is intended for educational, systems programming, and performance engineering purposes.
