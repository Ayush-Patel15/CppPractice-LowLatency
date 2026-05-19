/*
Now, A SPSC queue (Single Producer Single Consumer) - It's a fixed size ring buffer with two indices or pointers
- tail, used by push to add or write an element
- head, used for pop or remove an element

It has a Producer and a Consumer, so no need of CAS or so or mutex lock and unlock. Simple atomic - release and acquire works

- Producer: Owns the tail(write) and head(read)
- Consumer: Owns the head(write) and tail(read)

## Design Decisions Before Writing Code

Decision 1 — Fixed capacity, power of 2
// modulo — division is expensive
index = (index + 1) % N;
// bitmask — single AND instruction, N must be power of 2
index = (index + 1) & (N - 1);

Decision 2 — Who owns what
Producer owns: tail (write), head (read)
Consumer owns: head (write), tail (read)

Decision 3 — Memory ordering
Producer:
  buffer write     → no atomic needed (protected by release)
  tail.store()     → release (makes buffer write visible)
  head.load()      → acquire (sees consumer's latest head)

Consumer:
  buffer read      → no atomic needed (protected by acquire)
  head.store()     → release (makes consumption visible)
  tail.load()      → acquire (sees producer's latest tail)
*/


// Necessary headers
#include <iostream>
#include <thread>
#include <cassert>
#include <cstddef>
#include <atomic>

// The SPSC class
template<typename T, size_t N>
class SPSC{
    // Rule 1 - Multiple of 2, static assert checks
    static_assert(N >= 2, "N should be greater than or equal to 2");
    static_assert((N & (N-1)) == 0, "N should be a multiple of 2");

private:
    // Decalrations
    T buffer[N];
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};

public:
    // Method: To push a value using tail
    bool push(const T& val){
        // Read the tail and go to next
        size_t t = tail.load(std::memory_order_relaxed);
        size_t next = (t + 1) & (N - 1);

        // Check full or not
        if(next == head.load(std::memory_order_acquire)){
            return  false;
        }

        // Write
        buffer[t] = val;
        tail.store(next, std::memory_order_release);

        return true;
    }

    // Method: To pop a l value using head
    bool pop(T& val){
        // Read the head
        size_t h = head.load(std::memory_order_relaxed);

        // Check empty or not
        if(h == tail.load(std::memory_order_acquire)){
            return false;
        }

        // read the value
        val = buffer[h];
        head.store((h+1) & (N-1), std::memory_order_release);

        return true;
    }
};


// Use the SPSC queue
int main(){
    
    // Define the SPSC and threads
    SPSC<int, 1024> queue;

    // produver to push the value
    std::thread producer([&](){
        for(int i=0; i<= 10000; i++){
            while(!queue.push(i));
        }
    });

    // consumer to pop the value
    std::thread consumer([&](){
        int val;
        int count = 0;
        while(count <= 10000){
            if(queue.pop(val)){
                count++;
            }
        }
        std::cout << "Last popped count: " << count << " and value is: " << val << std::endl; 
    });

    // join the thread
    producer.join();
    consumer.join();

    return 0;
}