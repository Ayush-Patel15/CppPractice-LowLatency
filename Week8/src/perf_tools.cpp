/*
There are 3 perf tools, to measure the performance on the Linux systems: (Hardware dependent)

1). perf stats: It is used to get the performance stats of the entire program i.e. entire execution of the code. Mainly provides: 
total instructions, total cycles, cache misses, cache references, branch misses, branch instructions, and many more.
- IPC: Instruction / cycles i.e. no. of CPU cycles needed to complete 1 instruction. 
        > 3.0   → excellent — superscalar execution, CPU fully utilized
        2.0-3.0 → good
        1.0-2.0 → average — some stalls
        0.5-1.0 → memory bound — CPU waiting for data
        < 0.5   → severely memory bound or branch misprediction heavy

- Cache miss rate: Cache misses / Cache references i.e. number of times, it misses the L1, or L3 (Last level) cache, and goes to RAM
        < 1%    → excellent cache behavior
        1-5%    → acceptable
        5-10%   → investigate data layout
        > 10%   → serious cache problem — fix data structures first

- Branch miss rate: Branch misses / Branch Instructions i.e. number of times, it misses the branch prediction and got a penalty.
        < 1%    → excellent prediction
        1-3%    → acceptable
        > 5%    → consider branchless alternatives


2). perf record: It works by sampling - every N cycles i.e. the program sends an interrupt and stops at a instruction level, after every
N cycles, and note the instruction. And then, provides the numbers to analyze, like where or which function is utilizing the system - 
the most. It is not good for shorter burst time programs, as will not be able to collect enough samples. Ideally, either do multiple 
iteration runs, or heavy computation such that the program should be in running or execution state for 1-2 secs.

EXAMPLE:
# record with default sampling
perf record ./your_program

# record with higher frequency for short programs
perf record -F 99 ./your_program

# record with call graph for full stack traces
perf record -g ./your_program

3). pref report: This is more on the per instruction level i.e. it collects the data of perf record, then dig down deeper in each instruction
executed and percentage of CPU utilized by it or kept busy by it. 

Overhead  Command  Shared Object     Symbol
  45.23%  program  program           [.] SPSCQueue::push
  23.11%  program  program           [.] processOrder
  12.45%  program  libc.so           [.] malloc
   8.90%  program  program           [.] matchOrder
   5.12%  program  [kernel]          [k] copy_page

   -------------------------------------------------

Overhead    → percentage of samples in this function
Command     → which process
Shared Object → which binary/library
Symbol      → function name
[.]         → user space
[k]         → kernel space

-------------------------------------------------------------------------------------------------------------------------------

Annotated View — Instruction Level
Press Enter on a function in perf report to see instruction-level breakdown:
bash# or directly
perf annotate SPSCQueue::push
Output:
       │     push(const T& val):
  0.12 │       mov    rax, QWORD PTR [rdi+4104]    ← tail.load
  0.08 │       lea    rcx, [rax+1]
  0.04 │       and    rcx, 0x3ff
 45.11 │       cmp    rcx, QWORD PTR [rdi+4096]    ← head.load ← HOT
  0.23 │       je     full
  0.18 │       mov    DWORD PTR [rdi+rax*4], esi   ← buffer write
  0.09 │       mov    QWORD PTR [rdi+4104], rcx    ← tail.store
*/