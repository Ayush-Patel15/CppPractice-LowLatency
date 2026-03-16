/*
alignas - to align the particular struct or data packet as of 64 bytes i.e. equivalent to a single cache line. A single cache line feth
from a CPU takes 64 bytes of cache from L1, L2, or L3 cacheline
*/

#include <iostream>

// For a struct to be alignas
struct alignas(64) Counter{
    double counter_0;
    int counter_1;
    int counter_2;
    int counter_3;
    int counter_4;
};
// Total size of Counter will be 64 bytes: actual size + tail padding, because of alignas function

int main(){
    // Primitive datatype as alignas
    alignas(64) int i_counter = 10;

    std::cout << "Size of Counter: " << sizeof(Counter) << std::endl;
    std::cout << "Align of Counter: " << alignof(Counter) << std::endl;
    std::cout << "Size of i_counter: " << sizeof(i_counter) << std::endl;
}