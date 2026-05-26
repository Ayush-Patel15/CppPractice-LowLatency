#pragma once

#include <atomic>
#include <cstddef>

// The SPSC Queue class
template<typename T, size_t N>
class SPSCQueue{
private:
    // stack based buffer
    T buffer[N];

    // Head and tail
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};

public:
    // The push method
    bool push(const T& val){
        // Read the current tail, and next index
        size_t t = tail.load(std::memory_order_relaxed);
        size_t next = (t + 1) & (N - 1);

        // If full
        if(next == head.load(std::memory_order_acquire)){
            return false;
        }

        // Write the val
        buffer[t] = val;
        tail.store(next, std::memory_order_release);
        return true;
    }

    // The pop method
    bool pop(T& val){
        // current head read
        size_t h = head.load(std::memory_order_relaxed);

        // If it's empty or not
        if(h == tail.load(std::memory_order_acquire)){
            return false;
        }

        // Read the head
        val = buffer[h];
        head.store((h+1) & (N - 1), std::memory_order_release);
        return true;
    }
};