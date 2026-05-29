#pragma once

/*
This is similar to rdtsc, just has it's own CPU re-ordering protection. We don't need to use the _mm_lfence() function 
*/

#include <x86intrin.h>
#include <chrono>
#include <thread>
#include <cstdint>

// The class
class RDTSCP{
private:
    // Attributes
    double tsc_per_ns;
    uint64_t start_tsc{0};
    uint64_t end_tsc{0};

    // Inline function to return
    inline uint64_t rdtscp(){
        unsigned int core_id;
        return __rdtscp(&core_id);
    }

    // Method: To compute the cycles per ns
    double measureTSCfrequency(){
        // Start time
        auto s_ns = std::chrono::high_resolution_clock::now();
        uint64_t s_ts = rdtscp();
        // Sleep the thread
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // end time
        uint64_t e_ts = rdtscp();
        auto e_ns = std::chrono::high_resolution_clock::now();
        // diff in time
        uint64_t diff_ts = e_ts - s_ts;
        auto diff_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(e_ns - s_ns).count();
        return (double)diff_ts/(double)diff_ns;
    }

public:
    // Constructor
    explicit RDTSCP(): tsc_per_ns(measureTSCfrequency()) {};

    // Function to start the timer
    void start(){
        start_tsc = rdtscp();
    }

    // Function to stop the timer
    void stop(){
        end_tsc = rdtscp();
    }

    // Elapsed cycles
    uint64_t elapsedCycles(){
        return end_tsc - start_tsc;
    }

    // Elapsed Ns
    uint64_t elapsedNs(){
        return uint64_t((end_tsc - start_tsc) / tsc_per_ns);
    }
};