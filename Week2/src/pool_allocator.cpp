/*
A custom Pool allocator to solve the problem of new / malloc function. New / malloc is a time consuming allocation process.
However, in pool allocator - we take a piece of memory block at the start of the program. Then perform allocate() and 
deallocate() in O(1) time complexity.
*/

// header files
#include <iostream>
#include <cstddef>
#include <cassert>
#include <chrono>
#include <vector>


// The custom Pool allocator
class PoolAllocator{
private:
    // A FreeSlot struct - holding a pointer
    struct FreeSlot{
        FreeSlot* next;
    };

    // Attributes
    void* memory_block;
    FreeSlot* head;
    size_t slot_size;
    size_t capacity;

public:
    // Constructor
    explicit PoolAllocator(size_t slot_size, size_t capacity)
    : slot_size(slot_size), capacity(capacity){
        // Assset check
        assert(slot_size >= sizeof(FreeSlot));
        // Initialize a void memory block
        memory_block = ::operator new(slot_size * capacity);
        head = nullptr;
        // Create a free linked list of capacity blocks
        for(int i=(int)capacity-1; i>=0; i--){
            void* slot = (char*)memory_block + (i * slot_size);
            FreeSlot* node = (FreeSlot*)slot;
            node->next = head;
            head = node;
        }
    }

    // Destructor
    ~PoolAllocator() noexcept{
        ::operator delete(memory_block);
    }

    // copy constructor is not allowed ~ as shallow or deep copy, are of no use here
    PoolAllocator(const PoolAllocator&) = delete;

    // copy assignment is also not of any use
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    // The move constructor
    PoolAllocator(PoolAllocator&& other) noexcept
    : memory_block(other.memory_block), head(other.head), slot_size(other.slot_size), capacity(other.capacity){
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
    }

    // The move operator assignment
    PoolAllocator& operator=(PoolAllocator&& other) noexcept{
        if(this == &other) return *this;
        ::operator delete(memory_block);
        memory_block = other.memory_block;
        head = other.head;
        slot_size = other.slot_size;
        capacity = other.capacity;
        // Free up the existings
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
        return *this;
    }

    // The Allocate method
    void* allocate(){
        if(head == nullptr) return nullptr;
        FreeSlot* node = head;
        head = head->next;
        return (void*)node;
    }

    // The deallocate method
    void deallocate(void* ptr){
        if(ptr == nullptr) return;
        FreeSlot* node = (FreeSlot*)ptr;
        node->next = head;
        head = node;
    }

    // get capacity function
    size_t getCapacity(){
        return capacity;
    }

    // Get the slot size
    size_t getSize(){
        return  slot_size;
    }
};

//////////////////// BENCHMARKING TEST FOR POOL ALLOCATOR ///////////////////////////////////
struct Order{
    double price;
    int id;
    int quantity;
    bool status;
};

const int ITERATIONS = 5'000'000;

// Allocate using new
long long benchmarkNew(){
    auto start_time = std::chrono::high_resolution_clock::now();
    // Initialize and fill the vector
    std::vector<Order*> orders_list;
    orders_list.reserve(ITERATIONS);
    for(int i=0; i<ITERATIONS; i++){
        orders_list.push_back(new Order{100.5, 15141312, 50, true});
    }
    // Delete that order
    for(Order* o: orders_list){
        delete o;
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    return duration;
}


// Allocate using the pool allocator
long long benchmarkPool(){
    // Pre-initialize the pool
    PoolAllocator pool(sizeof(Order), ITERATIONS);
    auto start_time = std::chrono::high_resolution_clock::now();
    // Initialize and fill the vector
    std::vector<Order*> orders_list;
    orders_list.reserve(ITERATIONS);
    for(int i=0; i<ITERATIONS; i++){
        void* slot = pool.allocate();
        orders_list.push_back(new(slot) Order{100.5, 15141312, 50, true});
    }
    // Delete the order and deallocate
    for(Order* o: orders_list){
        o->~Order();
        pool.deallocate(o);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    return duration;
}

// The main function - to run
int main(){
    // warm - up run: for first time
    benchmarkNew();
    benchmarkPool();

    // The actual run
    long long time_n = benchmarkNew();
    long long time_p = benchmarkPool();

    // cout to the output screen
    std::cout << "For New operator: " << time_n << " microseconds\n";
    std::cout << "For Pool operator: " << time_p << " microseconds\n";
    std::cout << "Speed optimized by: " << (double)time_n / time_p << " x\n";

    return 0;
}