/*
Arena Allocator - It's a custom allocator to store dynamic-sized objects in O(1) allocation time complexity. Whereas, to deallocate - 
it just reset to the start of the memory, i.e. erase or delete all of the objects stored at once. It has a pointer called as 
bump pointer to return the address, from where the memory is free and can be used.
*/

// Headers
#include <cstdint>
#include <cassert>
#include <cstddef>
#include <chrono>
#include <iostream>
#include <new>


// Custom Arena allocator
class ArenaAllocator{
private:
    // Private class attributes
    void* memory_block;
    char* bump_ptr;
    char* memory_end;
    size_t total_size;

public:
    // Explicit constructor
    explicit ArenaAllocator(size_t total_size)
    : total_size(total_size){
        memory_block = ::operator new(total_size);
        bump_ptr = (char*)memory_block;
        memory_end = bump_ptr + total_size;
    }

    // Destructor
    ~ArenaAllocator() noexcept {
        ::operator delete(memory_block);
    }

    // Copy constructor = delete
    ArenaAllocator(const ArenaAllocator&) = delete;

    // Copy assignment operator = delete
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    // Helper to get the used space or memory
    size_t usedMemory() const {
        return size_t(bump_ptr - (char*)memory_block);
    }

    // Helper to get the free memory
    size_t freeMemory() const {
        return size_t(memory_end - bump_ptr);
    }

    // To allocate - memory
    void* allocate(size_t size, size_t alignment=alignof(std::max_align_t)){
        char* aligned = (char*)(((uintptr_t)bump_ptr + alignment - 1) & ~(alignment - 1));
        if(aligned + size > memory_end){
            return nullptr;
        }
        bump_ptr = aligned + size;
        return (void*)aligned;
    }

    // To reset - deallocate
    void reset(){
        bump_ptr = (char*)memory_block;
    }

    // The move constructor
    ArenaAllocator(ArenaAllocator&& other) noexcept
    : memory_block(other.memory_block), bump_ptr(other.bump_ptr), memory_end(other.memory_end), total_size(other.total_size){
        other.memory_block = nullptr;
        other.bump_ptr = nullptr;
        other.memory_end = nullptr;
        other.total_size = 0;
    }

    // The move assignment operator
    ArenaAllocator& operator=(ArenaAllocator&& other) noexcept {
        if(this == &other) return *this;
        ::operator delete(memory_block);
        memory_block = other.memory_block;
        bump_ptr = other.bump_ptr;
        memory_end = other.memory_end;
        total_size = other.total_size;
        // empty the other
        other.memory_block = nullptr;
        other.bump_ptr = nullptr;
        other.memory_end = nullptr;
        other.total_size = 0;
        return *this;
    }
};


// Using that arena allocator
struct Header{
    int mst_type;
    int sequence;
};

struct Payload{
    double price;
    int qty;
    int flags;
};

struct Result{
    double adjusted_price;
    bool valid;
};

// Constant
const int ITERATIONS = 100000;

// The main function
int main(){
    ArenaAllocator arena(1024 * 1024);      // 1MB arena allocation

    auto start_time = std::chrono::high_resolution_clock::now();
    // Iterate and assign memory
    for(int msg=0; msg < ITERATIONS; msg++){
        void* h = arena.allocate(sizeof(Header));
        void* p = arena.allocate(sizeof(Payload));
        void* r = arena.allocate(sizeof(Result));

        // Create the objects at defined memory location
        Header* header = new(h) Header{msg, 1};
        Payload* payload = new(p) Payload{100.5, 50, 1};
        Result* result = new(r) Result{100.10, 1};

        // Do some processing
        (void)header;
        (void)payload;
        (void)result;

        // Reset everthing at the end
        arena.reset();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    std::cout << "Time take in microseconds is: " << duration << " us\n";
}
