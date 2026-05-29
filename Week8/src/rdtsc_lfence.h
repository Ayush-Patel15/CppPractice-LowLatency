#pragma once

// The header
#include <emmintrin.h>
#include <x86intrin.h>
#include <cstdint>
#include <chrono>
#include <thread>

// The class
class RdtscTimer{
private:
    // private attributes
    double tsc_per_ns;
    uint64_t start_ts{0};
    uint64_t end_ts{0};

    // To get the rdtsc time
    inline uint64_t rdtsc(){
        return __rdtsc();
    }

    // private method
    double measureTSCfrequency(){
        auto start_ns = std::chrono::high_resolution_clock::now();
        uint64_t start_tsc = rdtsc();
        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        uint64_t end_tsc = rdtsc();
        auto end_ns = std::chrono::high_resolution_clock::now();
        // get the difference
        uint64_t diff_tsc = end_tsc - start_tsc;
        auto diff_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_ns - start_ns).count();
        return (double)diff_tsc / diff_ns;
    }

public:
    // Constructor
    explicit RdtscTimer(): tsc_per_ns(measureTSCfrequency()) {};

    // To start the timer
    void start(){
        _mm_lfence();
        start_ts = rdtsc();
    }

    // To stop the timer
    void stop(){
        end_ts = rdtsc();
        _mm_lfence();
    }

    // Getter for elapsed ns
    uint64_t elapsedNs(){
        return uint64_t((end_ts - start_ts) / tsc_per_ns);
    }

    // Getter for elapsed cycles
    uint64_t elapsedCycles(){
        return end_ts - start_ts;
    }

    // Getter for the private member
    double get_tsc_per_ns() const {
        return tsc_per_ns;
    }
};