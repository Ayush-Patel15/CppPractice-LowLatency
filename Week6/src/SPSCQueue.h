#pragma once

#include <cstddef>
#include <cassert>
#include <atomic>

// The SPSC Queue
template<typename T, size_t N>
class SPSCQueue{
    static_assert(N >= 2, "N should always be greater than or equal to 2");
    static_assert((N & (N - 1))==0, "N should be a power of 2");

private:
    T buffer[N];

    // For preventing false sharing
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};

public:
    // Constructor
    SPSCQueue() = default;

    // Destructor
    ~SPSCQueue() = default;

    // The copy constructor
    SPSCQueue(const SPSCQueue& other) = delete;

    // The copy assignment operator
    SPSCQueue& operator=(const SPSCQueue& other) = delete;

    // The move constructor
    SPSCQueue(SPSCQueue&& other) = delete;

    // The move assingment operator
    SPSCQueue& operator=(SPSCQueue&& other) = delete;

    // The push operation
    bool push(const T& val){
        size_t t = tail.load(std::memory_order_relaxed);
        size_t next = (t + 1) & (N - 1);
        // If the queue is full - false
        if(next == head.load(std::memory_order_acquire)){
            return false;
        }
        // Else, allowed to write
        buffer[t] = val;
        tail.store(next, std::memory_order_release);
        return true;
    }

    // The pop operation
    bool pop(T& val){
        size_t h = head.load(std::memory_order_relaxed);
        // If empty - return false
        if(h == tail.load(std::memory_order_acquire)){
            return false;
        }
        // If allowed, get the value
        val = buffer[h];
        head.store((h+1) & (N-1), std::memory_order_release);
        return true;
    }
};