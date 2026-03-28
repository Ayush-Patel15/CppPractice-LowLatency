/*
Custom fixed-size Pool allocator, using mmap to directly call from OS
*/

// Headers to include
#include <cstddef>
#include <sys/mman.h>
#include <cassert>
#include <cstring>

// Pool allocator class
class PoolAllocator{
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

public:
    // Explicit constructor
    explicit PoolAllocator(size_t slot_size, size_t capacity)
    : slot_size(slot_size), capacity(capacity){
        // An assert check
        assert(slot_size >= sizeof(FreeSlot));

        // Initialize the memory
        size_t total = slot_size * capacity;
        memory_block = mmap(
            nullptr,
            total,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
            -1,
            0
        );
        // if failed
        if(memory_block == MAP_FAILED){
            memory_block = mmap(
                nullptr,
                total,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1,
                0
            );
        }

        // Initialize the pages
        memset(memory_block, 0, total);
        // Assign head to each slot
        head = nullptr;
        for(int i=(int)capacity-1; i>=0; i--){
            void* slot = (char*)memory_block + (i * slot_size);
            FreeSlot* node = (FreeSlot*)slot;
            node->next = head;
            head = node;
        }
    }

    // Destructor
    ~PoolAllocator() noexcept{
        if(memory_block != nullptr){
            munmap(memory_block, slot_size * capacity);
        }
    }

    // Delete the copy constructor and copy assigment operator
    PoolAllocator(const PoolAllocator&) = delete;
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
        if(memory_block != nullptr){
            munmap(memory_block, slot_size * capacity);
        }
        memory_block = other.memory_block;
        head = other.head;
        slot_size = other.slot_size;
        capacity = other.capacity;
        // Steal from other
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
        return *this;
    }

    // Allocate function
    void* allocate() {
        if(head == nullptr) return nullptr;
        FreeSlot* slot = head;
        head = head->next;
        return (void*)slot;
    }

    // To deallocate
    void deallocate(void* ptr){
        if(ptr == nullptr) return;
        FreeSlot* slot = (FreeSlot*)ptr;
        slot->next = head;
        head = slot;
    }
};

