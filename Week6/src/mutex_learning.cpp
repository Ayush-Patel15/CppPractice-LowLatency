/*
Mutex is used for mutual exclusion i.e. to gain a locked access of a critical section (shared resource). Such that, at a time a single
process or a single thread can hold that critical section.

Mutex provides with memory barriers (happens-before, similar to atomic) and also performs a compare-and-swap (CAS) to get a critical section.
Therefore, for an uncontented or a single mutex lock and unlock, it still costs around 40-50 ns. While, atomic costs only 4-5 ns.
Therefore, it's always essential to decide - whether we need to use mutex or not.

Whereas, a contended mutex or two threads working for the same shared resource, costs even more. It is as follows - 

- Thread 1 is already working with the critical section
- Thread 2 comes to work, or ask for that critical section
- As, it was occupied - it throws or swap out the thread 2 to the OS
- After completion of the Thread 1 functionalities - it wakes up the Thread 2
- Wakes the thread 2 from OS and gets it to the ready queue
- Assign a core, as per the core allocation or scheduling technique
- Then, the Thread 2 resumes it work

There are two more consequences or disadvatages to mutex

- Cache destruction: For each context-switch between the threads, the hot path or the cache of that thread's execution gets deleted or replaced
by another cache or data. Therefore, for each time the thread 2 wakes up - it cache also needs to be reloaded to the registers.

- Priority Inversion: If Thread 2 is of the high-priority to work, but the Thread 1 has occupied the mutex - it will not release or switch the
context, mid execution i.e. it will first complete its execution --> then call mutex.unlock


## Types of multi-threaded process guarantee implementaions -

Wait-free:   every thread completes in a bounded number of steps → strongest guarantee, hardest to implement

Lock-free:   at least one thread always makes progress → practical guarantee, achievable for most structures

Obstruction-free: a thread makes progress if it runs alone → weakest, rarely sufficient for HFT
*/

#include <iostream>
#include <thread>
#include <mutex>

// Define the shared counter
int counter = 0;
std::mutex mtx;


// Function to increment
void increment(){
    for(int i=0; i<100000; i++){
        mtx.lock();
        counter++;
        mtx.unlock();
    }
}


// Call the threads from main
int main(){
    std::thread t1(increment);
    std::thread t2(increment);

    t1.join();
    t2.join();

    std::cout << "The counter's final value is: " << counter << std::endl;

    return 0;
}
