/*
Thread - Smallest unit of individual instruction, that can be performed independently, from the CPU or the core.

std::thread is a RAII based wrapper, arounf the OS native thread handler. And A thread can be in three following state:
- deafult: Safe to destroy or destruct
- joinable: can be joined, at completion and then destructed
- detached: Running in background independently.

FUNDAMENTAL RULE - Each thread created using std::thread, can be either joined or detached (before destruction). Violating this, will 
lead to std::terminate (hard kill of the thread or process). std::terminate, just performs a hard kill on the process, and it is gone
or lost forever.
*/

// Header files
#include <chrono>
#include <stdexcept>
#include <stop_token>
#include <thread>

// global data
int data = 15;

// Function
void demoFunction(){
    data++;
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

// the main function
int main(){
    // initialize a std::thread
    std::thread t(demoFunction);

    // If a exception occurs, before the join - the thread will terminated unexpectedly
    if(data != 15){
        throw std::runtime_error("Data is not equal to the expected value");
    }

    // Join it
    t.join();

    return 0;
}


/*
As, during a runtime exception, the thread is not destroyed cleanly. Therefore, we need to have a guard or a destructor around the
thread, to exit it cleanly, without leaving a trace or unexpected behaviour.
*/

// A thread guard class
class ThreadGuard{
private:
    std::thread& t;

public:
    // Constructor
    explicit ThreadGuard(std::thread& t): t(t) {};

    // Destructor
    ~ThreadGuard() noexcept{
        if(t.joinable()){
            t.join();
        }
    }

    // Copy contructor and copy assignment: Copy is not allowed in threads
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;

    // Moving a thread is allowed and legal - they are not related to a fixed memory, where they are defined
    // If we have a std::vector<std::thread>, and the vector reallocates - the threads will be moved safely to the
    // new reallocated location
};

int main(){
    std::thread t(demoFunction);
    ThreadGuard thread_guard(t);

    // A runtime exception - will destroy or call the destructor of ThreadGuard class - which will destroy the thread, after the join
    if(data != 16){
        throw std::runtime_error("Data is not equal to 15");
    }

    // Doesn't matter, we call the t.join() or not. It will safely, gets destructed.

    return 0;
}


/*
std::jthread - This jthread is exactly, the ThreadGuard class. It is under STL itself, and is safe to use, without any overhead.
It guarantees, the thread destruction - which will not lead to std::terminate.
*/

void safeFunction(){
    std::jthread t(demoFunction);

    // Even after a runtime exception, the destructor will be called safely
    if(data != 15){
        throw std::runtime_error("Data is not equal to 15");
    }
}

/*
std::jthread are capable of cooperative cancellations: i.e. a thread can be requested to stop, mid execution - if requried
*/

// Using the stop token
std::jthread trading_thread([](std::stop_token stp){
    // Run until not requested to stop
    while(!stp.stop_requested()){
        processMarketData();
        buildOrderBook();
        checkRMSlimits();
    }
});

// For requesting to stop
trading_thread.request_stop();

/*
There a std::stop_source mechanism also - used to control cooperative cancellations, on a group of threads. Let's say - 
we have 3 threads running with std::stop_source, then calling the func.request_stop() - will stop all of them, from execution
*/