/*
The RAII (Resource Allocation Is Initialisation) concept - Allocate the resource lifespan to its object lifespan,
to avoid memory leaks or fragmentations.

The 5 principles:
- Destructor
- The copy constructor
- The move constructor
- The copy assignment operator
- The move assignment operator
*/


#include <iostream>
#include <cstddef>

// Buffer class - resource binded to its object
class Buffer{
private:
    int* data;
    size_t size;

public:
    // Constructor
    explicit Buffer(size_t n)
    : data(new int[n]), size(n){}

    // Destructor
    ~Buffer() noexcept{
        delete[] data;
    }

    // The copy constructor
    Buffer(const Buffer& other)
    : data(new int[other.size]), size(other.size){
        std::copy(other.data, other.data+size, data);
    }

    // The copy assignment operator
    Buffer& operator=(const Buffer& other){
        if(this == &other) return *this;
        delete[] data;
        data = new int[other.size];
        size = other.size;
        std::copy(other.data, other.data+size, data);
        return *this;
    }

    // The move constructor
    Buffer(Buffer&& other) noexcept
    : data(other.data), size(other.size){
        other.data = nullptr;
        other.size = 0;
    }

    // The move assignment operator
    Buffer& operator=(Buffer&& other) noexcept{
        if(this == &other) return *this;
        delete[] data;
        data = other.data;
        size = other.size;
        other.data = nullptr;
        other.size = 0;
        return *this;
    }
};