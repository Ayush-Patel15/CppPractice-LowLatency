/*
Week 5 Task — Thread-Safe Flag With Atomic Ordering

What You're Building
A thread-safe one-shot flag — producer sets it once, consumer waits for it and reads associated data. 
Simple concept, every ordering choice must be deliberate and justified.

Part 1 — Implement the Flag
Build a ReadyFlag class with:
An std::atomic<bool> flag internally
A void set(int data) method — stores data, then signals ready
A int wait() method — spins until ready, then returns data
Every atomic operation must use an explicit non-default memory order
No memory_order_seq_cst allowed — you must choose the minimal correct ordering

Part 2 — Write the Test
Spawn two threads:
Thread 1 calls flag.set(42) after sleeping 10ms
Thread 2 calls flag.wait() and asserts the returned value is exactly 42
Run 1000 times in a loop — if ordering is wrong, it will eventually fail

*/

// headers
#include <iostream>
#include <cassert>
#include <chrono>
#include <atomic>
#include <thread>

// ReadyFlag class
class ReadyFlag{
private:
    // attributes
    std::atomic<bool> flag{false};
    int data;

public:
    // method: To set the data
    void set(int data){
        this->data = data;
        flag.store(true, std::memory_order_release);
        // flag.store(true, std::memory_order_relaxed);
    }

    // method: to read the return the data
    int wait(){
        while(!flag.load(std::memory_order_acquire));
        return this->data;
    }
};


int main(){
    for(int i=0; i<1000; i++){
        // Ready flag object
        ReadyFlag rf;
        // Initialize 2 threads
        std::thread t1([&](){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            rf.set(42);
        });
        // another thread to read
        std::thread t2([&]() {
            int res = rf.wait();
            assert(res == 42);
        });
        // Join the threads
        t1.join();
        t2.join();

    };
    std::cout << "All 1000 iterations passed" << "\n";
    return 0;
}