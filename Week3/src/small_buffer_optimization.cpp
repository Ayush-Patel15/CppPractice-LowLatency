/*
SBO: Small Buffer Optimizations - Store the data in the stack memory, till possible (or is possible to store). And then,
switch to Heap - for a larger memory space i.e. Let it be dynamic.
std::string uses the similar concept to store the small strings (up length 15 in the stack), and later on
dynamically shift it to Heap after the 15 chars. Because of this, stack space - the size of std::string is 32 bytes
instead of 8 bytes (a basic size of pointer).

- Lets design a stack based small vector
*/

// Include headers
#include <iostream>
#include <new>

// Small vector class
template<typename T, size_t n>
class smallvector{
private:
    alignas(T) char buffer[n * sizeof(T)];
    T* data;
    size_t size;
    size_t capacity;
    bool is_heap_allocated;

public:
    // Constructor
    explicit smallvector()
    : data(reinterpret_cast<T*>(buffer)), size(0), capacity(n), is_heap_allocated(false){};

    // Destructor
    ~smallvector() noexcept{
        // Destroy the elements first
        for(size_t i=0; i<size; i++){
            data[i].~T();
        }
        // Free up the heap based memory
        if(is_heap_allocated){
            ::operator delete(data);
        }
    }

    // Push back operation
    void push_back(const T& other){
        // If can store
        if(size < capacity){
            new(data + size) T(other);
            size++;
        }
        else{
            growToHeap();
            new(data + size) T(other);
            size++;
        }
    }

    // The grow to heap function
    void growToHeap(){
        // Get new capacity and new memory space
        size_t new_capacity = capacity * 2;
        T* new_data = (T*)::operator new(new_capacity * sizeof(T));
        // Move from old to new memory address
        for(size_t i=0; i<size; i++){
            new(new_data + i) T(std::move(data[i]));
            data[i].~T();
        }
        // delete the previously heap allocator data
        if(is_heap_allocated){
            ::operator delete(data);
        }
        // Assign the new variables to old
        capacity = new_capacity;
        data = new_data;
        is_heap_allocated = true;
    }
};


int main(){
    // For 8 push, it will use stack memory. For the 9th, will switch to heap
    smallvector<int, 8> vc;
    vc.push_back(1);
    vc.push_back(2);
    vc.push_back(3);
    vc.push_back(4);
    vc.push_back(5);
    vc.push_back(6);
    vc.push_back(7);
    vc.push_back(8);
    // Heap allocated after this
    vc.push_back(9);
    vc.push_back(10);
    vc.push_back(11);
    vc.push_back(12);
    std::cout << "\n";
    return 0;
}
