// benchmark: random access cost scaling with array size
#include "RdtscTimer.h"
#include "LatencyHistogram.h"
#include <vector>
#include <random>
#include <iostream>

template<typename ArrType>
void benchmarkRandomAccess(ArrType& arr, size_t size, const std::string& label){
    std::mt19937_64 rng{42};
    std::uniform_int_distribution<size_t> dist(0, size - 1);

    RdtscTimer timer;
    LatencyHistogram hist;

    // warmup
    for(int i = 0; i < 10'000; i++){
        volatile auto x = arr[dist(rng)];
        (void)x;
    }

    for(int i = 0; i < 100'000; i++){
        size_t idx = dist(rng);
        timer.start();
        volatile auto x = arr[idx];
        timer.end();
        (void)x;
        hist.record(timer.get_elapsed_ns());
    }

    hist.print(label);
}

int main(){
    std::vector<void*> small_arr(50'000, nullptr);      // fits in L2/L3
    std::vector<void*> large_arr(1'500'000, nullptr);   // exceeds L3

    benchmarkRandomAccess(small_arr, 50'000, "Small array (50K entries)");
    benchmarkRandomAccess(large_arr, 1'500'000, "Large array (1.5M entries)");
}