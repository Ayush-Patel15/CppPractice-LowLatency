/*
ABA is the most classical problem in multi-threaded systems with multiple producers (because of the race condition)
Let's say, there's a buffer (single index) with value A, in it

- Thread 1, reads the first value i.e. A
- Thread 2 comes, and updates or do the next write as B (Now, the buffer has B)
- Thread 2 again, updates the value from B to A (Now, the buffer has A)
- Thread 1 comes again or resumes and sees the value to be A again i.e. the expected value, and performs the CAS

However, the value has been updated to B, and then back to A, but that change is lost or not seen by the thread 1. This is the ABA problem
*/

////////////// Classical ABA problem with Lock-Free stack /////////////////////////
#include <atomic>


struct Node{
    int value;
    Node* next;
};

// atomic shared variable
std::atomic<Node*> head{nullptr};

// For the push operation
void push(Node* node){
    node->next = head.load(std::memory_order_relaxed);
    while(!head.compare_exchange_weak(
        node->next,
        node,
        std::memory_order_release,
        std::memory_order_relaxed
    ));
}

// For the pop operation
Node* pop(){
    Node* old_head = head.load(std::memory_order_acquire);
    while (old_head && !head.compare_exchange_weak(
        old_head,
        old_head->next,
        std::memory_order_release,
        std::memory_order_relaxed
    ));
    return old_head;
}

/*
Initial stack: A → B → C
head = &A

Thread 1: reads head = &A, reads A->next = &B
          (context switch — Thread 1 pauses here)

Thread 2: pops A  (head = &B)
Thread 2: pops B  (head = &C)
Thread 2: pushes A back (head = &A, but A->next is now garbage or &C)

Thread 1 resumes: CAS sees head == &A → succeeds
Thread 1 sets head = &B  ← B was already popped and freed head now points to freed memory undefined behavior, likely crash
*/
