#pragma once

// The header files
#include <cstdint>
#include <thread>
#include <x86intrin.h>
#include <emmintrin.h>
#include <chrono>


////////////// The RDTSC time Class
class RdtscTimer {
private:
    // attributes
    uint64_t start_ts{0};
    uint64_t end_ts{0};
    double tsc_per_ns;

    // The in-line function to get timestamp counter
    inline uint64_t rdtsc(){
        return __rdtsc();
    }

    // Measure the tsc per ns frequency
    double measureTscFrequency(){
        // Start the timer
        auto start_ns = std::chrono::high_resolution_clock::now();
        uint64_t start_tsc = rdtsc();
        // Sleep in between
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // end the timer
        uint64_t end_tsc = rdtsc();
        auto end_ns = std::chrono::high_resolution_clock::now();
        // get the diff and tsc per ns
        auto diff_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_ns - start_ns).count();
        uint64_t diff_ts = end_tsc - start_tsc;
        return (double)diff_ts / (double)diff_ns;
    }

public:
    // The constructor
    explicit RdtscTimer(): tsc_per_ns(measureTscFrequency()){};

    // Getter: To get the tsc per ns
    double get_tsc_per_ns() const{
        return tsc_per_ns;
    }

    // Method: the start function
    void start(){
        _mm_lfence();
        start_ts = rdtsc();
    }

    // Method: the end function
    void end(){
        end_ts = rdtsc();
        _mm_lfence();
    }

    // Method: to get the elasped cycles
    uint64_t get_elapsed_cycles() const{
        return end_ts - start_ts;
    }

    // Method: to get the elapsed time for ns
    uint64_t get_elapsed_ns() const{
        return (uint64_t)((double)(end_ts - start_ts) / tsc_per_ns);
    }
};