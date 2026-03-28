/*
mmap - stands for memory map, it talks with OS directly. Used to call for memory, directly from the OS.
Better thatn malloc, for assigning large memory blocks because - 
- No memory fragmentation
- No searching in free list
- No book-keeping
- No segmentation fault

mmap doesn't initialize the memory, on calling the function. It assigns the memory on page faults. As,
OS divides the memory into fixed-size pages, and then uses TLB to store the virtual memory to its physical memory (page address) address.

General size of pages is 4KB
General entries in TLB is 64 - 128
So, total memory in consideration is 64 * 4KB = 256KB

Therefore, use huge pages instead of 4KB pages.
Each page size = 2MB
TLB entries = 64
Total memory in consideration = 64 * 2MB = 128MB (Enough for allocators, used in HFT)
*/

// Allocate huge pages, and a TLB to map their address

// include files
#include <cstring>
#include <sys/mman.h>

// function to allocate the memory
void* allocate_huge_memory(size_t size){
    size_t page_size = 2 * 1024 * 1024;      // 2MB as page size
    size = (size + page_size - 1) & ~(page_size - 1);

    // call for memory from OS
    void* ptr = mmap(nullptr, size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,       // MAP_HUGETLB (for a TLB to map huge pages)
        -1,
        0
    );

    // If the memory call fails - fallback from huge tlb
    if(ptr == MAP_FAILED){
        void* ptr = mmap(
            nullptr,
            size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1, 
            0
        );
    }

    // To initialize the entire memory - tocuh each page
    memset(ptr, 0, size);

    return ptr;
};

// Function to de allocate the acquire space
void deallocate_huge_memory(void* ptr, size_t size){
    munmap(ptr, size);
    return;
};
