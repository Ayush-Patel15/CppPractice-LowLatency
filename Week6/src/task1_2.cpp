/*
Part 1 — Implement the SPSC Queue
Implement SPSCQueue<T, N> from scratch without looking at the Topic 2 code. It must have:
Fixed capacity ring buffer, N must be power of 2 enforced by static_assert
bool push(const T& val) — returns false if full
bool pop(T& val) — returns false if empty
Correct memory ordering on every atomic operation — no seq_cst allowed
False sharing prevention on head and tail
Rule of 5 applied correctly — copy deleted, move allowed

Part 2 — Correctness Stress Test
Spawn one producer and one consumer. Producer pushes 10,000,000 integers sequentially — 0, 1, 2, 3... Consumer pops and verifies:
Every value is received exactly once
Values arrive in order
No values are skipped or duplicated
Final count matches exactly 10,000,000
If any assertion fails — your memory ordering is wrong. Run 10 times. All must pass.
*/


// Header files
#include "SPSCQueue.h"
#include <iostream>
#include <thread>

// The main block
int main(){
    const int ITERATIONS = 10'000'000;

    // Stress testing it for 10 times
    for(int j=0; j<10; j++){
        SPSCQueue<int, 128> queue;

        // Initialize the producer thread
        std::thread producer([&](){
            for(int i=0; i<ITERATIONS; i++){
                while(!queue.push(i));
            }
        });

        // Start the consumer thread
        std::thread consumer([&](){
            int val;
            int expected = 0;
            int count = 0;
            while(count < ITERATIONS){
                if(queue.pop(val)){
                    assert(val == expected);
                    expected++;
                    count++;
                }
            }
            std::cout << "Final count, after popping all is: " << count << "\n" << std::endl;
        });

        // Join th threads
        producer.join();
        consumer.join();
    }

    return 0;
}