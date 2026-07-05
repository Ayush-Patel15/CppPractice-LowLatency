#pragma once

// Includes
#include <cassert>
#include <cstring>
#include <sys/mman.h>


/////////////////////// Pool Allocator Class - mmap based //////////////////////
class PoolAllocator{
private:
    // A Free slot pointer
    struct FreeSlot{
        FreeSlot* next;
    };

    // The member attributes
    void* memory_block;
    FreeSlot* head;
    size_t slot_size;
    size_t capacity;
    size_t total_size;

public:
    // Constructor
    explicit PoolAllocator(size_t slot_size, size_t capacity): slot_size(slot_size), capacity(capacity), total_size(slot_size * capacity){
        // assert check
        assert(slot_size >= sizeof(FreeSlot));

        // initialize memory
        memory_block = mmap(
            nullptr,
            total_size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0
        );

        // Assert check
        assert(memory_block != MAP_FAILED);

        // Pre-fault the pages
        memset(memory_block, 0, total_size);

        // Assign the head pointer
        head = nullptr;
        for(int i=(int)capacity-1; i>=0; i--){
            void* temp = (char*)memory_block + (i * slot_size);
            FreeSlot* slot = (FreeSlot*)temp;
            slot->next = head;
            head = slot;
        }
    }

    // The destructor
    ~PoolAllocator() noexcept{
        if(memory_block != nullptr){
            munmap(memory_block, total_size);
        }
    }

    // The copy constructor
    PoolAllocator(const PoolAllocator& other) = delete;

    // The copy assignment
    PoolAllocator& operator=(const PoolAllocator& other) = delete;

    // The move constructor
    PoolAllocator(PoolAllocator&& other) noexcept: 
        memory_block(other.memory_block), head(other.head), slot_size(other.slot_size), capacity(other.capacity), total_size(other.total_size){
            // Steal the resources
            other.memory_block = nullptr;
            other.head = nullptr;
            other.slot_size = 0;
            other.capacity = 0;
            other.total_size = 0;
        }

    // The move assignment operator
    PoolAllocator& operator=(PoolAllocator&& other) noexcept{
        if(this == &other) return *this;
        // First delete, then assign
        if(memory_block != nullptr){
            munmap(memory_block, total_size);
        }
        // Assign
        memory_block = other.memory_block;
        head = other.head;
        slot_size = other.slot_size;
        capacity = other.capacity;
        total_size = other.total_size;
        // Steal
        other.memory_block = nullptr;
        other.head = nullptr;
        other.slot_size = 0;
        other.capacity = 0;
        other.total_size = 0;
        return *this;
    }

    // The allocate function
    void* allocate(){
        if(head == nullptr) return nullptr;
        // Else, get the memory
        FreeSlot* slot = head;
        head = head->next;
        return (void*)slot;
    }

    // The deallocate function
    void deallocate(void* ptr){
        if(ptr == nullptr) return;
        // Else, get back to memory
        FreeSlot* slot = (FreeSlot*)ptr;
        slot->next = head;
        head = slot;
        return;
    }
};
