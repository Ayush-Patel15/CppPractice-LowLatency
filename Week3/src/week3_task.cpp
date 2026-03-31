/*
Part 1 — Two Pool Allocator Versions

Have both versions ready in the same file:
PoolAllocatorNew — uses ::operator new / ::operator delete
PoolAllocatorMmap — uses mmap with huge page attempt + fallback + mbind + pre-fault with memset
Both must be functionally identical — same interface, same free list logic, same Rule of 5.

Part 2 — Benchmark Both
Write a benchmark that does the following for both allocators:
Allocate 1,000,000 Order objects one at a time
Do a small amount of work on each object (write to a field)
Deallocate each object
Measure total time in nanoseconds
Run each benchmark 3 times and average the results.

Part 3 — Measure Page Faults
Run both versions under time command and compare:
time ./week3_task
Observe the sys time — higher sys time indicates more OS involvement, more page faults, more kernel activity. 
The new version should show higher sys time than the mmap pre-faulted version during steady-state operation.
*/

// Headers to include
#include <ctime>
#include <sys/mman.h>
#include <numaif.h>
#include <iostream>
#include <chrono>
#include <cassert>
#include <cstring>
#include <cerrno>
#include <new>


// Pool Allocator using the new / delete operator
class PoolAllocatorNew{
private:
    // slot structure
    struct FreeSlot{
        FreeSlot* next;
    };
    // attributes
    void* memory_block;
    FreeSlot* head;
    size_t slot_size;
    size_t capacity;

public:
    // explicit constructor
    explicit PoolAllocatorNew(size_t slot_size, size_t capacity)
    : slot_size(slot_size), capacity(capacity){
        // assert check
        assert(slot_size >= sizeof(FreeSlot));
        // Initialize memory, and iterate for head slot
        memory_block = ::operator new(slot_size * capacity);
        head = nullptr;
        for(int i=(int)capacity-1; i >= 0; i--){
            void* node = (char*)(memory_block) + (i * slot_size);
            FreeSlot* slot = (FreeSlot*)node;
            slot->next = head;
            head = slot;
        }
    }

    // Destructor
    ~PoolAllocatorNew() noexcept{
        ::operator delete(memory_block);
    }

    // Delete the copy constructor and copy assignment operator
    PoolAllocatorNew(const PoolAllocatorNew&) = delete;
    PoolAllocatorNew& operator=(const PoolAllocatorNew&) = delete;

    // The move constructor
    PoolAllocatorNew(PoolAllocatorNew&& other) noexcept
    : memory_block(other.memory_block), head(other.head), slot_size(other.slot_size), capacity(other.capacity){
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
    }

    // Move assignment operator
    PoolAllocatorNew& operator=(PoolAllocatorNew&& other) noexcept{
        if(this == &other) return *this;
        if(memory_block != nullptr){
            ::operator delete(memory_block);
        }
        memory_block = other.memory_block;
        head = other.head;
        slot_size = other.slot_size;
        capacity = other.capacity;
        // nullify
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
        return *this;
    }

    // The allocate function
    void* allocate(){
        if(head == nullptr) return nullptr;
        FreeSlot* slot = head;
        head = head->next;
        return (void*)slot;
    }

    // The deallocate function
    void deallocate(void* ptr){
        if(ptr == nullptr) return;
        FreeSlot* slot = (FreeSlot*)ptr;
        slot->next = head;
        head = slot;
    }
};


// Pool Allocator using the mmap, munmap
class PoolAllocatorMmap{
private:
    // Free slot struct
    struct FreeSlot{
        FreeSlot* next;
    };
    // attributes
    void* memory_block;
    FreeSlot* head;
    size_t slot_size;
    size_t capacity;
    size_t map_size;

public:
    // Explicit construct
    explicit PoolAllocatorMmap(size_t slot_size, size_t capacity)
    : slot_size(slot_size), capacity(capacity), map_size(slot_size * capacity){
        // Initialize with huge page size
        assert(slot_size >= sizeof(FreeSlot));
        memory_block = mmap(
            nullptr,
            map_size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
            -1,
            0
        );
        // If failed
        if(memory_block == MAP_FAILED){
            memory_block = mmap(
                nullptr,
                map_size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1,
                0
            );
        }
        // bind it to the first numa node - 0 is the numa node
        unsigned long mask = 1UL << 0;
        int result = mbind(
            memory_block,
            map_size,
            MPOL_BIND,
            &mask,
            sizeof(mask) * 8,
            0
        );
        if(result != 0){
            std::cerr << "mbind failed: " << strerror(errno) << "\n";
        }
        // Hit or touch each page - for initialisation
        memset(memory_block, 0, map_size);
        // Assign head
        head = nullptr;
        for(int i=(int)capacity-1; i>=0; i--){
            void* node = (char*)memory_block + (i * slot_size);
            FreeSlot* slot = (FreeSlot*)node;
            slot->next = head;
            head = slot;
        }
    }

    // Destructor
    ~PoolAllocatorMmap() noexcept{
        if(memory_block != nullptr){
            munmap(memory_block, map_size);
        }
    }

    // Copy constructor and copy assignment
    PoolAllocatorMmap(const PoolAllocatorMmap&) = delete;
    PoolAllocatorMmap& operator=(const PoolAllocatorMmap&) = delete;

    // Move constructor
    PoolAllocatorMmap(PoolAllocatorMmap&& other) noexcept
    : memory_block(other.memory_block), head(other.head), slot_size(other.slot_size), capacity(other.capacity), map_size(other.map_size ){
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
        other.map_size = 0;
    }

    // The move assignment operator
    PoolAllocatorMmap& operator=(PoolAllocatorMmap&& other) noexcept{
        if(this == &other) return *this;
        if(memory_block != nullptr){
            munmap(memory_block, map_size);
        }
        memory_block = other.memory_block;
        head = other.head;
        slot_size = other.slot_size;
        capacity = other.capacity;
        map_size = other.map_size;
        // grab resources from other
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
        other.map_size = 0;
        return *this;
    }

    // The allocate function
    void* allocate(){
        if(head == nullptr) return nullptr;
        FreeSlot* slot = head;
        head = head->next;
        return (void*)slot;
    }

    // The de-allocate function
    void deallocate(void* ptr){
        if(ptr == nullptr) return;
        FreeSlot* slot = (FreeSlot*)ptr;
        slot->next = head;
        head = slot;
        return;
    }
};


// Order struct
struct Order{
    double price;
    int order_id;
    int qty;
    bool flag;
};

// Iterations count & macros
const int ITERATIONS = 1'000'000;
using Clock = std::chrono::high_resolution_clock;
using nanoseconds = std::chrono::nanoseconds;

// benchmark function
long long benchmarkNew(){
    PoolAllocatorNew pool(sizeof(Order), 1);
    auto start = Clock::now();
    for(int i=0; i<ITERATIONS; i++){
        void* raw = pool.allocate();
        Order* o = new(raw) Order{100.0, i, 40, true};
        o->price = 110.50;
        o->~Order();
        pool.deallocate(o);
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<nanoseconds>(end - start).count();
}

// benchmark function
long long benchmarkMmap(){
    PoolAllocatorMmap pool(sizeof(Order), 1);
    auto start = Clock::now();
    for(int i=0; i<ITERATIONS; i++){
        void* raw = pool.allocate();
        Order* o = new(raw) Order{100.0, i, 40, true};
        o->price = 110.50;
        o->~Order();
        pool.deallocate(o);
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<nanoseconds>(end - start).count();
}

// The main function
int main(){
    // Warm-up runs
    benchmarkNew();
    benchmarkMmap();

    // three runs each
    long long n1 = benchmarkNew(),  n2 = benchmarkNew(),  n3 = benchmarkNew();
    long long m1 = benchmarkMmap(), m2 = benchmarkMmap(), m3 = benchmarkMmap();

    std::cout << "\n=== PoolAllocatorNew ===\n";
    std::cout << "Run 1: " << n1 << " ns\n";
    std::cout << "Run 2: " << n2 << " ns\n";
    std::cout << "Run 3: " << n3 << " ns\n";
    std::cout << "Avg:   " << (n1+n2+n3)/3 << " ns\n";

    std::cout << "\n=== PoolAllocatorMmap ===\n";
    std::cout << "Run 1: " << m1 << " ns\n";
    std::cout << "Run 2: " << m2 << " ns\n";
    std::cout << "Run 3: " << m3 << " ns\n";
    std::cout << "Avg:   " << (m1+m2+m3)/3 << " ns\n";

    return 0;
}