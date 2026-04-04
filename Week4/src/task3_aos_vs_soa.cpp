/*
Part 3 — AoS vs SoA Layout
You have 1,000,000 orders. Sum all prices.
Write two versions:

Version A — AoS layout (std::vector<Order> where Order has price, quantity, flags)
Version B — SoA layout (separate arrays for prices, quantities, flags)

Measure both with -O3 -march=native. Observe the difference.
*/

// headers
#include <ctime>
#include <iostream>
#include <vector>
#include <chrono>

// constans
const int N = 1'000'000;
using Clock = std::chrono::high_resolution_clock;
using ns = std::chrono::nanoseconds;

// Order struct - for Arraty Of Structs
struct Order{
    double price;
    int quantity;
    bool flag;
};

// Struct - with arrays
struct Orders{
    double* prices;
    int* quantities;
    bool* flags;
};


// benchmark the array of structs
long long benchmark_aos(std::vector<Order>& aos_orders){
    auto start = Clock::now();
    double result = 0.0;
    for(int i=0; i<N; i++){
        result += aos_orders[i].price;
    }
    volatile double sink = result;
    (void)sink;
    auto end = Clock::now();
    return std::chrono::duration_cast<ns>(end - start).count();
}


// benchmark the struct of arrays
long long benchmark_soa(Orders& soa_orders){
    auto start = Clock::now();
    double result = 0.0;
    for(int i=0; i<N; i++){
        result += soa_orders.prices[i];
    }
    volatile double sink = result;
    (void)sink;
    auto end = Clock::now();
    return std::chrono::duration_cast<ns>(end - start).count();
}


// the main function
int main(){
    // Initialize a AoS
    std::vector<Order> aos_orders(N);
    for(int i=0; i<N; i++){
        aos_orders[i] = {i + 50.0, i, true};
    }
    // Initialize a SoA
    Orders soa_orders;
    soa_orders.prices = new double[N];
    soa_orders.quantities = new int[N];
    soa_orders.flags = new bool[N];
    for(int i=0; i<N; i++){
        soa_orders.prices[i] = i + 50.0;
        soa_orders.quantities[i] = i;
        soa_orders.flags[i] = true;
    }
    
    // Call warm-up benchmark
    benchmark_aos(aos_orders);
    benchmark_soa(soa_orders);

    // Actual 3 run call
    long long t1, t2, t3;
    long long t4, t5, t6;

    // To get the average
    t1 = benchmark_aos(aos_orders);
    t2 = benchmark_aos(aos_orders);
    t3 = benchmark_aos(aos_orders);
    t4 = benchmark_soa(soa_orders);
    t5 = benchmark_soa(soa_orders);
    t6 = benchmark_soa(soa_orders);

    long long avg_aos = (t1+t2+t3)/3;
    long long avg_soa = (t4+t5+t6)/3;

    // ── Results ───────────────────────────────────────────
    std::cout << "\n=== AoS vs SoA Layout (N=" << N << ") ===\n";
    std::cout << "AoS sum prices: " << avg_aos << " ns\n";
    std::cout << "SoA sum prices: " << avg_soa << " ns\n";
    std::cout << "Winner: " << (avg_soa < avg_aos ? "SoA" : "AoS") << "\n";

    // Clean - ups
    delete [] soa_orders.prices;
    delete [] soa_orders.quantities;
    delete [] soa_orders.flags;

    return 0;
}