/*
Week 7 Task — Pin two threads to specific cores, measure latency of your SPSC queue with and without pinning.
Measure round-trip latency — producer pushes a timestamp, consumer pops it and measures the difference:
Use std::chrono::high_resolution_clock or rdtsc for timing
Measure 1,000,000 round trips
Record p50, p99, p999 latencies

1. Pin producer thread to core 2
2. Pin consumer thread to core 3
3. Run your SPSC queue latency benchmark (from Week 6 Part 3)
4. Measure p50, p99, p999 WITHOUT pinning
5. Measure p50, p99, p999 WITH pinning
6. Compare the two — show the difference
*/

// Headers
#include "spscqueue.h"
#include <cassert>
#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>

// Method to pin a thread to a core
void pinThreadToCore(int core_number){
    // cpu set, and it's set affinity
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(core_number, &cpu_set);

    // Thread assignment
    int result = pthread_setaffinity_np(
        pthread_self(),
        sizeof(cpu_set),
        &cpu_set
    );

    // If not able to assign
    if(result != 0){
        throw std::runtime_error("Failed with a result code of " + std::to_string(result));
    }
    return;
}

// Method to set a priority, on scheduler
void setRealTimePriority(int rt_priority){
    // Assign priority
    struct sched_param param;
    param.sched_priority = rt_priority;

    // Assign the priority
    int result = pthread_setschedparam(
        pthread_self(),
        SCHED_FIFO,
        &param
    );

    // If not successful
    if(result != 0){
        throw std::runtime_error("Scheduling failed with a result code of " + std::to_string(result));
    }
    return;
}

// Method to set up a thread
void setUpAThread(int core_number, int rt_priority, bool use_rt=false){
    // setup
    pinThreadToCore(core_number);
    if(use_rt){
        setRealTimePriority(rt_priority);
    }

    // Verification on thread pinning
    cpu_set_t cpu_set;
    pthread_getaffinity_np(
        pthread_self(),
        sizeof(cpu_set),
        &cpu_set
    );
    assert(CPU_ISSET(core_number, &cpu_set));

    // Verification of sched policy
    if(use_rt){
        struct sched_param param;
        int policy;
        pthread_getschedparam(
            pthread_self(),
            &policy,
            &param 
        );
        assert(policy == SCHED_FIFO);
        assert(param.sched_priority == rt_priority);
    }
    return;
}

// The main function
int main(){
    // The SPSC Queue
    SPSCQueue<long long, 64> queue;
    const int ITERATIONS = 1'000'000;
    std::vector<long long> latencies;
    latencies.reserve(ITERATIONS);

    // The producer thread
    std::jthread producer([&](){
        setUpAThread(5, 90, false);
        // Run in a while, to push
        for(int i=0; i<ITERATIONS; i++){
            long long ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            while(!queue.push(ts));
        }
    });

    // The consumer thread
    std::jthread consumer([&](){
        setUpAThread(7, 90, false);
        // Pop the ts
        long long ts;
        for(int i=0; i<ITERATIONS; i++){
            while(!queue.pop(ts));
            // Get the diff
            long long now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            latencies.push_back(now - ts);
        }
    });

    // Join the threads
    producer.join();
    consumer.join();

    // Sort the latencies
    std::sort(latencies.begin(), latencies.end());

    // cout the latencies
    std::cout << "The p50: " << latencies[ITERATIONS * 0.50] << std::endl;
    std::cout << "The p99: " << latencies[ITERATIONS * 0.99] << std::endl;
    std::cout << "The p999: " << latencies[ITERATIONS * 0.999] << std::endl;

    return 0;
}
