/*
Part 3 — Latency Measurement
Measure round-trip latency — producer pushes a timestamp, consumer pops it and measures the difference:

Use std::chrono::high_resolution_clock or rdtsc for timing
Measure 1,000,000 round trips
Record p50, p99, p999 latencies
*/

// Headers
#include "SPSCQueue.h"
#include "mutexQueue.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <chrono>
#include <thread>

// the const
const int ITERATIONS = 1'000'000;

// for benchmarking the spsc queue
void benchmark_spsc(){
    // Define a queue
    SPSCQueue<long long, 1024> queue;
    
    // Define a vector
    std::vector<long long> latencies;
    latencies.reserve(ITERATIONS);

    // Producer thread
    std::thread producer([&](){
        for(int i=0; i<ITERATIONS; i++){
            long long ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            while(!queue.push(ts));
        }
    });

    // Consumer thread
    std::thread consumer([&]() {
        long long ts;
        for(int i=0; i < ITERATIONS; i++){
            while(!queue.pop(ts));
            // Get the current
            long long now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            latencies.push_back(now - ts);
        }
    });

    // Join the threads
    producer.join();
    consumer.join();

    // Get the latencies
    std::sort(latencies.begin(), latencies.end());

    // The p50, p99, and p999 
    std::cout << "The p50: " << latencies[ITERATIONS * 0.50] << " ns\n";
    std::cout << "The p99: " << latencies[ITERATIONS * 0.99] << " ns\n";
    std::cout << "The p999: " << latencies[ITERATIONS * 0.999] << " ns\n";

    return;
}


// For benchmarking the mutex queue
void benchmark_mutex(){
    // vectur and mutex
    MUTEXQueue<long long> queue(1024);
    std::vector<long long> latencies;
    latencies.reserve(ITERATIONS);

    // The producer
    std::thread producer([&](){
        for(int i=0; i<ITERATIONS; i++){
            long long ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            while(!queue.push(ts));
        }
    });

    // The consumer
    std::thread consumer([&]() {
        long long ts;
        for(int i=0; i<ITERATIONS; i++){
            while(!queue.pop(ts));
            // Get now and diff
            long long now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            latencies.push_back(now - ts);
        }
    });

    // Join the threads
    producer.join();
    consumer.join();

    // Latencies of p50, p99, p999
    std::sort(latencies.begin(), latencies.end());

    std::cout << "The p50: " << latencies[ITERATIONS * 0.50] << " ns\n";
    std::cout << "The p99: " << latencies[ITERATIONS * 0.99] << " ns\n";
    std::cout << "The p999: " << latencies[ITERATIONS * 0.999] << " ns\n";

    return;
}


int main(){
    // Run it multiple times
    for(int i=0; i<5; i++){
        benchmark_spsc();
        benchmark_mutex();
        std::cout << "\n";
    }

    return 0;
}
