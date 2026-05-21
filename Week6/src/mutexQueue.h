#pragma once

// Headers
#include <mutex>
#include <queue>
#include <cstddef>

// The mutex based queue class
template<typename T>
class MUTEXQueue{
private:
    std::queue<T> buffer;
    std::mutex mtx;
    size_t capacity;

public:
    // The constructor
    explicit MUTEXQueue(int capacity): capacity(capacity){};

    // The destructor
    ~MUTEXQueue() = default;

    // The RAII principles
    MUTEXQueue(const MUTEXQueue& other) = delete;
    MUTEXQueue& operator=(const MUTEXQueue& other) = delete;
    MUTEXQueue(MUTEXQueue&& other) = delete;
    MUTEXQueue& operator=(MUTEXQueue&& other) = delete;

    // The push operation
    bool push(const T& val){
        std::lock_guard<std::mutex> lock(mtx);
        if(buffer.size() >= capacity){
            return false;
        }
        buffer.push(val);
        return true;
    }

    // The pop operation
    bool pop(T& val){
        std::lock_guard<std::mutex> loc(mtx);
        if(buffer.empty()){
            return false;
        }
        val = buffer.front();
        buffer.pop();
        return true;
    }
};
