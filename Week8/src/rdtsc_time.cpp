/*
rdtsc stands for Read Time Stamp Counter - It is a hardware based counter of a x86 based architecture system, which returns the current
CPU counter of the system. It is extremely fast i.e. returns the counter in 2-3 CPU cycles i.e. near to 1 ns only. This returns the value
in CPU units, and not in nanoseconds. We need to convert it to nanoseconds, by a custom formula. 
rdtsc    ; loads TSC into EDX:EAX (high 32 bits : low 32 bits)

Compared to std::chrono or clock(), which internally calls the clock_gettime() function. And this is the actual time in nanoseconds, but is
a vDSO (Virtual Dynamic Shared Object) - which is sent or stored locally (cached) manner in the process memory (PCB). But, it quire time
expensive. Takes near about 30-50 ns. For a small burst time program, like a 60 ns, the overhead of monitoring the time, using the std::chrono
or so is not good or recommended. As, majority of the overhead - comes from the time computing formula (std::chrono)

It's essential to note that rdtsc can reorder the CPU instructions, so always use -
- _mm_lfence (for stopping the re-ordering)
- use the rdtscp, instead of rdtsc

rdtsc vs clock_gettime — The Complete Comparison
clock_gettime():
  Cost:           ~20-30ns per call
  Unit:           nanoseconds directly
  Monotonic:      yes
  Cross-core:     yes (synchronized across cores)
  Overhead:       significant for sub-100ns measurements
  Portability:    Linux/POSIX standard

rdtsc:
  Cost:           ~0.3-1ns per call
  Unit:           CPU cycles (convert to ns yourself)
  Monotonic:      yes (on modern CPUs with constant_tsc)
  Cross-core:     yes (on modern CPUs with nonstop_tsc)
  Overhead:       negligible
  Portability:    x86/x86-64 only
*/

// headers
#include <x86intrin.h>
#include <iostream>
#include <cstdint>

// The rdtsc function
inline uint64_t rdtsc(){
    return __rdtsc();
}

// the main function
int main(){
    // Count the CPU coutners, utilized
    uint64_t start = rdtsc();
    int total = 0;
    for(int i=0; i<1'000'000; i++){
        total += (i & 1);
    }
    uint64_t end = rdtsc();
    std::cout << "The difference is: " << end - start << " units" << std::endl;
    return 0;
}