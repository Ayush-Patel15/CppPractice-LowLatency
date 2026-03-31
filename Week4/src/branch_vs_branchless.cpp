/*
On conditional statements - The CPU tries to predict the section of code to run next, and even before the condition is evaluated.
It computes certain code in a branch. It the condition is valid - branch prediction is correct. But, if the condition is invalid - 
the branch is mispredicted, and hence - will need to flush the instructions computed and flush the pipe. This mispredeiction takes
5 - 7 nanoseconds to compute the correct branch code. Therefore, either try to write a branch-less condition check or predict the 
branch before hand. 
And the CPU has a Branch History Table (BHT), which maintains the cache for a branch i.e. tries to build a patter to dynamically 
detect the branch.

- Write branch and branchless versions of a same function i.e. add if the number > 128. And, see the difference
*/

#include <iostream>
#include <random>
#include <algorithm>
#include <cstring>
#include <chrono>

// macros
using Clock = std::chrono::high_resolution_clock;
using nanosecons = std::chrono::nanoseconds;

// constant
const int N = 10'000'000;

// Branchy way function
long long benchmark_branchy(int* data, long long* result){
    auto start = Clock::now();
    long long sum = 0;
    for(int i=0; i<N; i++){
        if(data[i] > 128){
            sum += data[i];
        }
    }
    *result = sum;
    auto end = Clock::now();
    return std::chrono::duration_cast<nanosecons>(end - start).count();
};

// Branch-less ternary operator function
long long benchmak_brancless_ternary(int* data, long long* result){
    auto start = Clock::now();
    long long sum = 0;
    for(int i=0; i<N; i++){
        sum += (data[i] > 128) ? data[i] : 0;
    }
    *result = sum;
    auto end = Clock::now();
    return std::chrono::duration_cast<nanosecons>(end - start).count();
};

// Branch-less bit manipulation
long long benchmark_branchless_bitwise(int* data, long long* result){
    auto start = Clock::now();
    long long sum = 0;
    for(int i=0; i<N; i++){
        int mask = -((int)(data[i] > 128));
        sum += (data[i] & mask);
    }
    *result = sum;
    auto end = Clock::now();
    return std::chrono::duration_cast<nanosecons>(end - start).count();
};

// Branchy, but predictable by sorting function
long long benchmark_branch_predictable(int* sorted_data, long long* result){
    auto start = Clock::now();
    long long sum = 0;
    for(int i=0;i<N; i++){
        if(sorted_data[i] > 128){
            sum += sorted_data[i];
        }
    }
    *result = sum;
    auto end = Clock::now();
    return std::chrono::duration_cast<nanosecons>(end - start).count();
};

// The main function - for benchmarking
int main(){
    // random data — unpredictable branches
    int* data = new int[N];
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    for(int i = 0; i < N; i++)
        data[i] = dist(rng);

    // sorted data — predictable branches
    int* data_sorted = new int[N];
    std::copy(data, data + N, data_sorted);
    std::sort(data_sorted, data_sorted + N);

    long long r1, r2, r3, r4;

    // warmup
    benchmark_branchy(data, &r1);
    benchmak_brancless_ternary(data, &r2);
    benchmark_branchless_bitwise(data, &r3);
    benchmark_branch_predictable(data_sorted, &r4);

    // three runs each
    long long b1=0, b2=0, b3=0, b4=0;
    for(int run = 0; run < 3; run++){
        b1 += benchmark_branchy(data, &r1);
        b2 += benchmak_brancless_ternary(data, &r2);
        b3 += benchmark_branchless_bitwise(data, &r3);
        b4 += benchmark_branch_predictable(data_sorted, &r4);
    }

    std::cout << "\n=== Branch Prediction Benchmark (N=" << N << ") ===\n";
    std::cout << "Branchy unpredictable: " << b1/3 << " ns\n";
    std::cout << "Branchless ternary:    " << b2/3 << " ns\n";
    std::cout << "Branchless bitmask:    " << b3/3 << " ns\n";
    std::cout << "Branchy predictable:   " << b4/3 << " ns\n";

    delete[] data;
    delete[] data_sorted;
    return 0;
}