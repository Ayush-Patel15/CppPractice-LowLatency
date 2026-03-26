/*
Task — benchmarking both allocators against new and each other. 
You have to built two allocators - Pool Allocator and Arena Allocator. Now you prove with numbers that they're faster than new, 
and understand where each one wins. This is your first multi-allocator benchmark.
*/

// Header files
#include <cstdint>
#include <ctime>
#include <iostream>
#include <cstddef>
#include <cassert>
#include <chrono>
#include <new>
#include <vector>


////////////////// POOL ALLOCATOR //////////////////
class PoolAllocator{
private:
    struct FreeSlot{
        FreeSlot* next;
    };
    // attributes
    void* memory_block;
    FreeSlot* head;
    size_t slot_size;
    size_t capacity;

public:
    // Constructor
    explicit PoolAllocator(size_t slot_size, size_t capacity)
    : slot_size(slot_size), capacity(capacity){
        assert(slot_size >= sizeof(FreeSlot));
        // Initialize
        memory_block = ::operator new(slot_size * capacity);
        head = nullptr;
        // Iterate
        for(int i=(int)capacity-1; i>=0; i--){
            void* slot = (char*)memory_block + (i * slot_size);
            FreeSlot* node = (FreeSlot*)slot;
            node->next = head;
            head = node;
        }
    }

    // Desctructor
    ~PoolAllocator() noexcept {
        ::operator delete(memory_block);
    }

    // Copy constructor
    PoolAllocator(const PoolAllocator&) = delete;

    // Copy assignment operator
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    // Move constructor
    PoolAllocator(PoolAllocator&& other) noexcept
    : memory_block(other.memory_block), head(other.head), slot_size(other.slot_size), capacity(other.capacity){
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
    }

    // Move assignment operator
    PoolAllocator& operator=(PoolAllocator&& other) noexcept{
        if(this == &other) return *this;
        ::operator delete(memory_block);
        memory_block = other.memory_block;
        head = other.head;
        slot_size = other.slot_size;
        capacity = other.capacity;
        // Free the other
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
        return *this;
    }

    // Allocate
    void* allocate(){
        if(head == nullptr) return nullptr;
        FreeSlot* slot = head;
        head = head->next;
        return (void*)slot;
    }

    // Deallocate
    void deallocate(void* ptr){
        if(ptr == nullptr) return;
        FreeSlot* slot = (FreeSlot*)ptr;
        slot->next = head;
        head = slot;
        return;
    }
};


///////////////// ARENA ALLOCATOR /////////////////
class ArenaAllocator{
private:
    // attributes
    void* memory_block;
    char* bump_ptr;
    char* memory_end;
    size_t capacity;

public:
    // Constructor
    explicit ArenaAllocator(size_t capacity)
    : capacity(capacity){
        memory_block = ::operator new(capacity);
        bump_ptr = (char*)memory_block;
        memory_end = bump_ptr + capacity;
    }

    // Destuctor
    ~ArenaAllocator() noexcept {
        ::operator delete(memory_block);
    }

    // Copy constructor
    ArenaAllocator(const ArenaAllocator&) = delete;

    // Copy assignment operator
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    // Move constructor
    ArenaAllocator(ArenaAllocator&& other) noexcept
    : memory_block(other.memory_block), bump_ptr(other.bump_ptr), memory_end(other.memory_end), capacity(other.capacity){
        other.memory_block = nullptr;
        other.bump_ptr = nullptr;
        other.memory_end = nullptr;
        other.capacity = 0;
    }

    // Move assignment operator
    ArenaAllocator& operator=(ArenaAllocator&& other) noexcept{
        if(this == &other) return *this;
        ::operator delete(memory_block);
        memory_block = other.memory_block;
        bump_ptr = other.bump_ptr;
        memory_end = other.memory_end;
        capacity = other.capacity;
        // Remove other
        other.memory_block = nullptr;
        other.bump_ptr = nullptr;
        other.memory_end = nullptr;
        other.capacity = 0;
        return *this;
    }

    // Allocate
    void* allocate(size_t size, size_t alignment=alignof(std::max_align_t)){
        char* aligned = (char*)(((uintptr_t)bump_ptr + alignment - 1) & ~(alignment - 1));
        if(aligned + size > memory_end){
            return nullptr;
        }
        bump_ptr = aligned + size;
        return (void*)aligned;
    }

    // Deallocate
    void reset(){
        if(memory_block == nullptr) return;
        bump_ptr = (char*)memory_block;
        return;
    }
};

///////////////////////////////////////// BENCHMARKING ///////////////////////////////
struct Order{
    double price;
    int order_id;
    int quantity;
    bool flag;
};

struct Header{
    int msg;
    int seq;
};

struct Payload{
    double price;
    int qty;
    bool flag;
};

struct Result{
    double adjusted_price;
    bool status;
};


// Constants
const int ITERATION = 1000000;
const int BURST_SIZE = 10000;
const int BURST_COUNT = 1000;

// Macro to use
using Clock = std::chrono::high_resolution_clock;
using Micro = std::chrono::nanoseconds;

// Single alloc and delete
long long benchmark_new_1(){
    auto start = Clock::now();
    for(int i=0; i<ITERATION; i++){
        Order* o = new Order{100.0, i, 40, 1};
        o->price = 100.50;
        delete o;
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<Micro>(end - start).count();
}

// Burst alloc and Burst free
long long benchmark_new_2(){
    std::vector<Order*> orders;
    orders.reserve(BURST_SIZE);
    auto start = Clock::now();
    for(int b=0; b<BURST_COUNT; b++){
        for(int i=0; i<BURST_SIZE; i++){
            orders.push_back(new Order{100.50, i, 40, 1});
        }
        for(Order* order: orders){
            delete order;
        }
        orders.clear();
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<Micro>(end - start).count();
}

// Single alloc and free, for Pool allocator
long long benchmark_pool_1(){
    PoolAllocator pool(sizeof(Order), 1);
    auto start = Clock::now();
    for(int i=0; i<ITERATION; i++){
        void* raw = pool.allocate();
        Order* o = new(raw) Order{100.0, i, 40, 1};
        o->price = 100.50;
        o->~Order();
        pool.deallocate(o);
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<Micro>(end - start).count();
}

// Burst alloc and burst free for Pool Allocator
long long benchmark_pool_2(){
    PoolAllocator pool(sizeof(Order), BURST_SIZE);
    std::vector<Order*> orders;
    orders.reserve(BURST_SIZE);
    auto start = Clock::now();
    for(int b=0; b<BURST_COUNT; b++){
        for(int i=0; i<BURST_SIZE; i++){
            void* raw = pool.allocate();
            orders.push_back(new(raw) Order{100.0, i, 40, 1});
        }
        for(Order* order: orders){
            order->~Order();
            pool.deallocate(order);
        }
        orders.clear();
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<Micro>(end - start).count();
}


// new on mixed-size objects
long long benchmark_new_3(){
    auto start = Clock::now();
    for(int i=0; i<ITERATION; i++){
        Header* h = new Header{i, 1};
        Payload* p = new Payload{100.50, 40, 1};
        Result* r = new Result{100.10, 1};
        (void)h; (void) p; (void)r;
        delete h;
        delete p;
        delete r;
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<Micro>(end - start).count();
}

// Arena allocator on mixed-size objects
long long benchmark_arena_1(){
    size_t arena_size = (sizeof(Header) + sizeof(Payload) + sizeof(Result) + 64) * ITERATION;
    ArenaAllocator arena(arena_size);
    auto start = Clock::now();
    for(int i=0; i<ITERATION; i++){
        void* h = arena.allocate(sizeof(Header), alignof(Header));
        void* p = arena.allocate(sizeof(Payload), alignof(Payload));
        void* r = arena.allocate(sizeof(Result), alignof(Result));
        new(h) Header{i, 1};
        new(p) Payload{100.50, 40, 1};
        new(r) Result{100.10, 1};
        arena.reset();
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<Micro>(end - start).count();
}

// Pool vs Arena for same Order object
long long benchmark_pool_4(){
    PoolAllocator pool(sizeof(Order), 1);
    auto start = Clock::now();
    for(int i=0; i<ITERATION; i++){
        void* raw = pool.allocate();
        Order* o = new(raw) Order{100.50, i, 40, 1};
        (void)o;
        o->~Order();
        pool.deallocate(o);
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<Micro>(end - start).count();

}

// Pool vs Arena for same Order object
long long benchmark_arena_4(){
    ArenaAllocator arena(sizeof(Order) + 64);
    auto start = Clock::now();
    for(int i=0; i<ITERATION; i++){
        void* raw = arena.allocate(sizeof(Order), alignof(Order));
        Order* o = new(raw) Order{100.50, i, 40, 1};
        (void)o;
        arena.reset();
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<Micro>(end - start).count();
}


/////////////////////// MAIN ///////////////////////////
int main(){
    // warm-up call
    benchmark_new_1();
    benchmark_new_2();
    benchmark_new_3();
    benchmark_pool_1();
    benchmark_pool_2();
    benchmark_pool_4();
    benchmark_arena_1();
    benchmark_arena_4();

    std::cout << "\n=== Benchmark 1: Single alloc/free x" << ITERATION << " ===\n";
    std::cout << "new/delete: " << benchmark_new_1() << " nanoseconds\n";
    std::cout << "pool: " << benchmark_pool_1() << " nanoseconds\n";

    std::cout << "\n=== Benchmark 2: Burst alloc/free ("<< BURST_COUNT << " bursts x " << BURST_SIZE << ") ===\n";
    std::cout << "new/delete: " << benchmark_new_2() << " nanoseconds\n";
    std::cout << "pool: " << benchmark_pool_2() << " nanoseconds\n";

    std::cout << "\n=== Benchmark 3: Mixed sizes x" << ITERATION << " ===\n";
    std::cout << "new/delete: " << benchmark_new_3() << " nanoseconds\n";
    std::cout << "arena: " << benchmark_arena_1() << " nanoseconds\n";

    std::cout << "\n=== Benchmark 4: Pool vs Arena x" << ITERATION << " ===\n";
    std::cout << "pool: " << benchmark_pool_4() << " nanoseconds\n";
    std::cout << "arena: " << benchmark_arena_4() << " nanoseconds\n";

    return 0;
}