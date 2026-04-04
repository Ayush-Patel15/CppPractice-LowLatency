/*
Part 1 — Branchless Order Filter
You have an array of 10,000,000 orders. Each order has a price and a quantity. You want to sum the total value (price × quantity) of all 
orders where price > 100.0. Write three versions:

Version A — branchy if/else
Version B — branchless ternary
Version C — branchless bitmask

Run each version on:
Random prices (unpredictable branches)
Sorted prices (predictable branches)

That gives you 6 total measurements. Record all six.
*/

// include statements
#include <algorithm>
#include <iostream>
#include <random>
#include <chrono>
#include <vector>


// namespace
using Clock = std::chrono::high_resolution_clock;
using ns = std::chrono::nanoseconds;
// const iterations
const int N = 10'000'000;

// Order struct
struct Order{
    double price;
    double quantity;
};

// Version A - branchy if-else
std::pair<long long, long long> branchy_if_else(std::vector<Order>& orders){
    auto start = Clock::now();
    long long total = 0;
    for(int i=0; i<N; i++){
        if(orders[i].price > 100.0){
            total += (long long)(orders[i].price * orders[i].quantity);
        }
    }
    auto end = Clock::now();
    return {std::chrono::duration_cast<ns>(end - start).count(), total};
}

// Version B - branchless ternary
std::pair<long long, long long> branchless_ternary(std::vector<Order>& orders){
    auto start = Clock::now();
    long long total = 0;
    for(int i=0; i<N; i++){
        total += orders[i].price > 100.0 ? (long long)(orders[i].price * orders[i].quantity) : 0LL;
    }
    auto end = Clock::now();
    return {std::chrono::duration_cast<ns>(end - start).count(), total};
}

// Version A - branchless bitwise
std::pair<long long, long long> branchless_bitwise(std::vector<Order>& orders){
    auto start = Clock::now();
    long long total = 0;
    for(int i=0; i<N; i++){
        long long mask = -((long long)orders[i].price > 100.0);
        long long curr = (long long)(orders[i].price * orders[i].quantity);
        total += curr & mask;
    }
    auto end = Clock::now();
    return {std::chrono::duration_cast<ns>(end - start).count(), total};
}


// The main function
int main(){
    // Create a order vector
    std::vector<Order> orders(N);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> prices_dist(50.0, 200.0);
    std::uniform_real_distribution<double> quantity_dist(1.0, 100.0);

    // Iterate and create order struct
    for(int i=0; i<N; i++){
        orders[i] = {prices_dist(rng), quantity_dist(rng)};
    }

    // Copy a sorted version
    std::vector<Order> sorted_orders = orders;
    std::sort(sorted_orders.begin(), sorted_orders.end(), 
        [](const Order& a, const Order& b){
            return a.price < b.price;
        }
    );

    // Warmp-up calls
    branchy_if_else(orders);
    branchless_ternary(orders);
    branchless_bitwise(orders);
    branchy_if_else(sorted_orders);
    branchless_ternary(sorted_orders);
    branchless_bitwise(sorted_orders);

    // Actual calls and store the result
    auto [d1, t1] = branchy_if_else(orders);
    auto [d2, t2] = branchless_ternary(orders);
    auto [d3, t3] = branchless_bitwise(orders);
    auto [d4, t4] = branchy_if_else(orders);
    auto [d5, t5] = branchless_ternary(orders);
    auto [d6, t6] = branchless_bitwise(orders);

    // ── Results ────────────────────────────────────────────
    std::cout << "\n=== Random Data (unpredictable) ===\n";
    std::cout << "Branchy if/else:     " << d1 << " ns\n";
    std::cout << "Branchless ternary:  " << d2 << " ns\n";
    std::cout << "Branchless bitmask:  " << d3 << " ns\n";

    std::cout << "\n=== Sorted Data (predictable) ===\n";
    std::cout << "Branchy if/else:     " << d4 << " ns\n";
    std::cout << "Branchless ternary:  " << d5 << " ns\n";
    std::cout << "Branchless bitmask:  " << d6 << " ns\n";
}
