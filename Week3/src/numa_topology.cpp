/*
NUMA - stands for Non Uniform Memory Access. In modern computers, cores are grouped to a particular socket, and
assigned a local RAM i.e. each socket group has its own local RAM. 

Socket 0                    Socket 1
┌─────────────────┐         ┌─────────────────┐
│   CPU Core 0-11 │         │   CPU Core 12-23│
│                 │         │                 │
│   L1/L2/L3      │         │   L1/L2/L3      │
│   Cache         │         │   Cache         │
└────────┬────────┘         └────────┬────────┘
         │                           │
    ┌────┴────┐                 ┌────┴────┐
    │  RAM 0  │                 │  RAM 1  │
    │  32GB   │◄───QPI Link────►│  32GB   │
    └─────────┘                 └─────────┘

- Memory access on local RAM (socket 0 to RAM 0) = 60 ns
- Memory access for remote RAM(socket 0 to RAM 1) = 120 ns (2 x cost)

NUMA NODE ==> SOCKET + RAM

Therefore, it is preferred to set or use the initialised memory of the same numa node, where our process thread is working.
Thread is pinned or startd on Numa Node 0 --> So, the memory should also be used from RAM0 (belongs to Node 0)

- Use mbind() with mmap() - to bind the node number

- mbind() works on virtaul memory addresses
*/

// Headers
#include <sys/mman.h>
#include <numaif.h>
#include <cstring>
#include <numa.h>

// Function to use mmap and mbind
void mmap_mbind(size_t size, int numa_node){
    // Call for memory
    void* ptr = mmap(
        nullptr,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    // mbind to the numa node 0
    unsigned long mask = 1UL << numa_node;
    int result = mbind(
        ptr,                             // the pointer address: start address
        size,                              // total size for the virtual memory
        MPOL_BIND,                        // Strict binding to the numa node
        &mask,                           // The numa node number
        sizeof(mask) * 8,              // mask ib bits, not in bytes (therefore multiplied by 8)
        0                                // To perform certain action on the existing data on virtual memory
    );

    // After binding - call the memset
    memset(ptr, 0, size);
};
