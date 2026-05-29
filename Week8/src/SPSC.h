#pragma once

#include <cstddef>
#include <cassert>
#include <atomic>

/////// The SPSC Queue ///////////////
template<typename T, size_t N>
class SPSCQueue{
    static_assert(N >= 2, "N should be greater than or equal to 2");
    static_assert((N & (N - 1)) == 0, "N should be a power of 2");

private:
    // attributes
    T buffer[N];
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};

public:
    // The push operation
    bool push(const T& val){
        // Load the tail
        size_t t = tail.load(std::memory_order_relaxed);
        size_t next = (t + 1) & (N - 1);
        // If the buufer is full
        if(next == head.load(std::memory_order_acquire)){
            return false;
        }
        // Else, write and store
        buffer[t] = val;
        tail.store(next, std::memory_order_release);
        return true;
    }

    // The pop operation
    bool pop(T& val){
        // load the head
        size_t h = head.load(std::memory_order_relaxed);
        // If empty
        if(h == tail.load(std::memory_order_acquire)){
            return false;
        }
        val = buffer[h];
        head.store((h+1)&(N-1), std::memory_order_release);
        return true;
    }
};