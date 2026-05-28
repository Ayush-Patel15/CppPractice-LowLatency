/*
CACHEGRIND

While perf is more of a hardware tool to check on cache misses. The Cachegrind (part of the valgrind lib) is a software based simulator, 
to simulate the cache behavior using the CPU cache lines. 
perf:        uses real hardware counters — fast, approximate
cachegrind:  simulates cache — slow (10-50x), exact

# compile with debug symbols
g++ -std=c++20 -O2 -pthread -g -o program src/program.cpp

# run under cachegrind
valgrind --tool=cachegrind ./program

# output goes to cachegrind.out.PID file
ls cachegrind.out.*

--------------------------------------------------------
==12345== Cachegrind, a cache and branch-prediction profiler
==12345== I   refs:      500,000,000   ← instruction references
==12345== I1  misses:         45,000   ← L1 instruction cache misses
==12345== LLi misses:          1,200   ← last level instruction misses
==12345==
==12345== D   refs:      300,000,000   ← data references (reads+writes)
==12345== D1  misses:     15,000,000   ← L1 data cache misses
==12345== LLd misses:      2,000,000   ← last level data cache misses
==12345==
==12345== D1  miss rate:        5.0%   ← L1 data miss rate
==12345== LLd miss rate:        0.67%  ← last level miss rate

------------------------------------------------------------------------------------------

Cachegrind's Cache Model
By default cachegrind simulates:
L1:  32KB, 8-way associative, 64 byte cache lines
L2:  not simulated separately
LL:  8MB, 16-way associative, 64 byte cache lines

You can configure it to match your actual hardware:
# match your i5-13420H specs from lscpu
valgrind --tool=cachegrind \
  --I1=32768,8,64   \   # L1 instruction: 32KB, 8-way, 64B lines
  --D1=49152,12,64  \   # L1 data: 48KB, 12-way, 64B lines
  --LL=12582912,16,64 \ # L3: 12MB, 16-way, 64B lines
  ./program


---------------------------------------------------------------------------------------------------------------------------------
BRANCH PREDICTION MISS RATE

cachegrind also simulates branch prediction. Add --branch-sim=yes:
valgrind --tool=cachegrind --branch-sim=yes ./program

Additional output:
==12345== Branches:       100,000,000
==12345== Mispredicts:      5,000,000   ← 5% miss rate
==12345== Bc miss rate:          5.0%
And per-line in cg_annotate:
Bc          Bcm    ← branch count, branch mispredicts
100,000,000  5,000,000  if(orders[i].price > 100.0)

-------------------------------------------------------------------------------------------------------------------------------
CALLGRIND

A related tool in the Valgrind suite — callgrind records which functions call which and how much time each call graph path costs:

valgrind --tool=callgrind ./program
callgrind_annotate callgrind.out.*

Output:
Ir                function
500,000,000       main
300,000,000         SPSCQueue::push
150,000,000           std::atomic::load
100,000,000         SPSCQueue::pop
50,000,000           std::atomic::store

Shows the complete call hierarchy with instruction counts. Useful for understanding which call paths are most expensive — 
not just which functions, but which paths through the program reach them.
*/