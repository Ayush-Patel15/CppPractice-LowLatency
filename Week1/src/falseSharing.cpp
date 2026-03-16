/*
Core 1          Core 2
  L1 cache        L1 cache
  L2 cache        L2 cache
       \             /
        L3 cache (shared)
              |
             RAM

False Sharing - When two differet threads on two different cores, tries to write to two different variables, that live inside
the same 64 byte cache line. Then, one has to wait for the other to free that cache line.
*/

#include <iostream>
#include <cstddef>
#include <thread>
#include <chrono>

// A bad counter version
struct SharedCounter{
    int counter_a;
    int counter_b;
};

// A good counter version
struct PaddedCounter{
    alignas(64) int counter_a;
    alignas(64) int counter_b;
};

// Static const
const int iterations = 100'000'000;

// A fuction to perform the iterations, and see the effect
template <typename T>
long long benchmarking(T& Counter){
    auto startTime = std::chrono::high_resolution_clock::now();

    // Lambda function thread for counter_a
    std::thread t1([&](){
        for(int i=0; i<iterations; i++){
            Counter.counter_a++;
        }
    });

    // Lambda function for thread for counter_b
    std::thread t2([&](){
        for(int i=0;i<iterations; i++){
            Counter.counter_b++;
        }
    });

    // Joint the threads
    t1.join();
    t2.join();

    auto endTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
}

// Assert checks
static_assert(sizeof(PaddedCounter::counter_a) == 4, "Counter a is not of 4 bytes");
static_assert(offsetof(PaddedCounter, counter_b) == 64, "counter b (of padded) is not padded to 64 bytes");
static_assert(offsetof(SharedCounter, counter_b) != 64, "counter b (of shared) is not at 64 bytes");

int main(){
    // Initialize both
    SharedCounter sh_counter;
    PaddedCounter pd_counter;
    sh_counter.counter_a = sh_counter.counter_b = 0;
    pd_counter.counter_a = pd_counter.counter_b = 0;

    // Call the functions
    long long sh_time = benchmarking(sh_counter);
    long long pd_time = benchmarking(pd_counter);

    // Compare
    std::cout << "Shared counter (falseSharing): " << sh_time << std::endl;
    std::cout << "Padded counter (Good version): " << pd_time << std::endl;
    std::cout << "Speed comparison: " << (float)sh_time / pd_time << std::endl;
}