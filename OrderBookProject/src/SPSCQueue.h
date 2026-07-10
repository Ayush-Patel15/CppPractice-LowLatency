# pragma once

// Include headers
#include <cstddef>
#include <cstdint>
#include <atomic>

// The SPSC class
template<typename T, size_t N>
class SPSCQueue{
private:
    // assert checks
    static_assert(N >= 2, " N shou;d be greater than or equal to 2");
    static_assert((N & (N - 1)) == 0, "N should be a multiple of 2");

    T buffer[N];
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};

public:
    // The constructor
    SPSCQueue() = default;

    // The destructor
    ~SPSCQueue() = default;

    // The copy constructor
    SPSCQueue(const SPSCQueue& other) = delete;

    // The move constructor
    SPSCQueue(SPSCQueue&& other) = delete;

    // The copy assignment
    SPSCQueue& operator=(const SPSCQueue& other) = delete;

    // The move assignment
    SPSCQueue& operator=(SPSCQueue&& other) = delete;

    // The push operation
    bool push(const T& val){
        size_t t = tail.load(std::memory_order_relaxed);
        size_t next = (t + 1) & (N - 1);
        // Check for full or not
        if(next == head.load(std::memory_order_acquire)){
            return false;
        }
        // Else, store and move
        buffer[t] = val;
        tail.store(next, std::memory_order_release);
        return true;
    }

    // The pop operation
    bool pop(T& val){
        size_t h = head.load(std::memory_order_relaxed);
        // If empty
        if(h == tail.load(std::memory_order_acquire)){
            return false;
        }
        // else get
        val = buffer[h];
        head.store((h+1)&(N-1), std::memory_order_release);
        return true;
    }
};
