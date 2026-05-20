/*
The solution to the ABA problem is a tagged pointer (kind of version controlling, over the nodes or data)

Pack a version counter alongside the pointer. The CAS checks both pointer AND version — 
even if the pointer returns to the same address, the version is different.
*/

#include <atomic>
#include <cstdint>

// A lock-free tagged ptr stack
template<typename T>
class LockFreeTaggedStack{
private:
    // A node struct
    struct Node{
        T value;
        Node* next;
        Node(const T& value): value(value), next(nullptr) {};
    };

    // A tagged ptr struct
    struct TaggedPtr{
        Node* ptr;
        uint64_t tag;

        // operator overloading
        bool operator==(const TaggedPtr& other) const {
            return (other.ptr == ptr) && (other.tag == tag);
        }
    };

    // Atomic variable
    std::atomic<TaggedPtr> head{{nullptr, 0}}; 

public:
    // The push operation
    void push(const T& val){
        Node* node = new Node(val);
        TaggedPtr old_head = head.load(std::memory_order_relaxed);
        TaggedPtr new_head;
        do{
            node->next = old_head.ptr;
            new_head = {node, old_head.tag + 1};
        } while(!head.compare_exchange_weak(
            old_head,
            new_head,
            std::memory_order_release,
            std::memory_order_relaxed
        ));
    }

    // The pop operation
    bool pop(T& val){
        TaggedPtr old_head = head.load(std::memory_order_acquire);
        TaggedPtr new_head;
        do{
            if(!old_head.ptr) return false;
            new_head = {old_head.ptr->next, old_head.tag + 1};
        } while(
            !head.compare_exchange_weak(
                old_head,
                new_head,
                std::memory_order_release,
                std::memory_order_acquire
            )
        );
        val = old_head.ptr->value;
        delete [] old_head.ptr;
        return true;
    }
};


/*
Initial:  head = {&A, tag=0}

Thread 1 reads: old_head = {&A, tag=0}
Thread 1 pauses.

Thread 2 pops A: head = {&B, tag=1}
Thread 2 pops B: head = {&C, tag=2}
Thread 2 pushes new node at &A: head = {&A, tag=3}

Thread 1 resumes CAS:
  expected = {&A, tag=0}
  actual   = {&A, tag=3}
  tag mismatch → CAS FAILS ✓
  Thread 1 reloads head, retries correctly and then succeeds
*/