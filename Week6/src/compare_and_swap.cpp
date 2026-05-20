/*
In SPSC - because of a single producer, we had atomicity around the write operation. But, when two threads try to update or write the same
index of a buffer, we need an atomic operation - which updates the value, if it was not changed from the last read. That is called Compare-and-Swap

What CAS does - 

if(*location == expected){
    *location = new_value
    return true   // success
} else {
    expected = *location  // load current value into expected
    return false          // failure — try again
}

There are two types of CAS in cpp:
- compare_exchange_strong: Returns fail, if it fails i.e. has a strong mechanism or strict mechanism
- compare_exchange_weak: Returns fail, even it the expected == *location

So, the rule says - if we are trying to CAS in a loop, go with weak so, it will retry on a fail. And, if we need the fail response, i.e. it
means something to the system - then use the weak command
*/

// headers to include
#include <atomic>

std::atomic<int> val{5};
int expected = 5;
int desired  = 10;

// attempts to change val from 5 to 10
bool success = val.compare_exchange_strong(expected, desired);
// bool success = val.compare_exchange_weak(expected, desired);


///////////////////////// THE LOOP RETRY ////////////////////////////////////////////
void push(std::atomic<int>& counter, int increment){
    int expected = counter.load(std::memory_order_relaxed);
    while(!counter.compare_exchange_weak(
              expected,           // updated to current on failure
              expected + increment,
              std::memory_order_release,   // success ordering
              std::memory_order_relaxed))  // failure ordering
    {
        // expected automatically updated to current value
        // loop retries with fresh expected
    }
}