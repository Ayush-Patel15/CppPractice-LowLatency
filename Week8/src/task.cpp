/*
Replace chrono With rdtsc
Take your Week 6 SPSC latency benchmark. Replace std::chrono::high_resolution_clock with your RdtscTimer or RDTSCP class. Run both versions and record:
chrono p50, p99, p999
rdtsc  p50, p99, p999

The rdtsc numbers should be lower — measurement overhead is reduced.
*/

// Headers
#include "rdtsc_lfence.h"
#include "SPSC.h"
#include <x86intrin.h>
#include <sched.h>
#include <string>
#include <thread>
#include <vector>
#include <pthread.h>
#include <iostream>
#include <algorithm>
#include <cstdint>


// Method: To pin the thread to the core
void pinThreadToCore(int core_number){
    // cpu set
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_number, &cpuset);
    // pin to thread
    int result = pthread_setaffinity_np(
        pthread_self(), 
        sizeof(cpuset),
        &cpuset
    );
    // If not success
    if(result != 0){
        std::cout << "Pin to Thread is failed with code: " << std::to_string(result) << std::endl;
    }
    return;
}


// The main function
int main(){
    // constants
    SPSCQueue<uint64_t, 1024> queue;
    RdtscTimer timer;
    const int ITERATIONS = 1'000'000;
    std::vector<uint64_t> latencies;
    latencies.reserve(ITERATIONS);

    // producer thread
    std::jthread producer([&](){
        pinThreadToCore(2);
        // push
        for(int i=0; i<ITERATIONS; i++){
            uint64_t ts = __rdtsc();
            while(!queue.push(ts));
        }
    });

    // consumer thread
    std::jthread consumer([&]() {
        pinThreadToCore(3);
        // pop
        uint64_t pushed_ts;
        for(int i=0; i<ITERATIONS; i++){
            while(!queue.pop(pushed_ts));
            // After the pop
            uint64_t now = __rdtsc();
            uint64_t cycles = now - pushed_ts;
            uint64_t tsc_ns = (uint64_t)(cycles / timer.get_tsc_per_ns());
            latencies.push_back(tsc_ns);
        }
    });

    // Join the threads
    producer.join();
    consumer.join();

    // Sort the latencies
    std::sort(latencies.begin(), latencies.end());

    // The percentiles
    std::cout << "The 50th percentile: " << latencies[ITERATIONS * 0.50] << std::endl;
    std::cout << "The 99th percentile: " << latencies[ITERATIONS * 0.99] << std::endl;
    std::cout << "The 99.9th percentile: " << latencies[ITERATIONS * 0.999] << std::endl;

    return 0;
}
