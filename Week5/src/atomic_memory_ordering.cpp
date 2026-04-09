/*
The compiler and CPU reorder the instructions based on there code structure - to optimize the execution. To avoid that, and take the
control in the hands of programmer - atomic is used.

std::atomic used for stopping or removing the Compiler and CPU optimizations, and memory reordering. It basically provides 3 things - 

- Atomicity i.e. performing operations on an individual or at an individualize level
- Happens-before guaranteed, if used memory ordering accordingly
- Thread safety for shared objects or data

There are different types of ordering possible - 

- std::memory_order_relaxed: Doesn't care about memory ordering, just provides atomicity and thread-safety
- std::memory_order_release: Work as a barrier for all write or store operations. All the write operations should be done before (happens-before), this
- std::memory_order_acquire: Work as a barrier for all read or load operations. All the read opeartion should be done after this.
- std::memory_order_acq_rel: Used to modify a value i.e. it do acquire the memory --> read it --> wirte/modify it --> then release the memory
- std::memory_order_seq_cst: Most expensive and highest time consuming check - that gurantees all the three properties all atomic

On the compiler level - the release and acquire compiles to mov operation (on assembly). While, the seq_cst leads to mfence (which blocks
the entire memory block at the OS level).


HAPPENS-BEFORE : It states that the instruction happened before, will be seen by the after instructions. In simple terms - 
If A happens-before B. B can see all the changes or writes done by A. And it is guranteed to do so, if A happens-before B.
And it also follows transitivity i.e. A happens-before B, and B happens before C --> Then, A happens before C also.
No happens-before means no synchronization across, the shared object or data.


TO AVOID THE DATA RACE CONDITION, AND UNDEFINED BEHAVIOUR ON MEMORY

We should have thread safety - to avoid data race and undefined behaviour on the shared memory, across various threads. To achieve this,
we have 3 methods or ways:

- Single threaded program - always executes line-by-line
- Using threads, and thread.join() - to synchronize different threads
- Using atmoic and memory ordering to guarantee the happens-before principle.
*/

// Include headers
#include <atomic>
#include <iostream>

int main(){
    int x;
    // Initialize a atomic variable
    std::atomic<bool> flag{false};

    // Write the data, and store 
    x = 42;                                                     // (A) : write operation on shared data
    flag.store(true, std::memory_order_release);           // (B) : lock or use release to lock the memory - before all writes

    // Acquire and read
    while(!flag.load(std::memory_order_acquire));             // (C): acquire - before reading to guarantee happens-before
    std::cout << x << "\n";                                     // (D): Guaranteed to be 42, always

    return 0;
}