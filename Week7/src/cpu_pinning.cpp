/*
CPU Pinning - Also known as CPU affinity, It is used to pin or allocate a core to a particular thread only.
OS schedules, and preempts the threads in between their executions for load balancing, thermal control, and other reasons.
But, because of this contex switching of threads - it increases the latency, because of:
- L1, L2 cache get reset, and the data needs to be reloaded, and read via the hot path
- TLB got cleared i.e. the TLB entries get destructed
- The Branch History Table (BHT), gets cleared up also

All, this 3 needs to be warmed up again - for the same thread to now execute it. Therefore, in HFT's we try to stop this CPU pinning
and assign a single process or functionality to a single thread or core (as a Single Responsibility Principle).

This CPU pinning is specific or say platform dependent. In Linux, systems - there's a bitwise mask to perform cpu pinning (cpu_set_t)
And the macros, it provides are - 
- CPU_ZERO(&cpu_set): To clear all the cpu bits, nothing assigned
- CPU_SET(nth, &cpu_set): To assign the nth cpu core to a thread
- CPU_CLR(nth, &cpu_set): To clear the nth cpu core, from a thread
- CPU_ISSET(nth, &cpu_set): To check the nth cpu core is set or not
- CPU_COUNT(&cpu_set): To get the count of set bits 
*/


//////////////////////////////// TO Assign a CPU CORE TO A THREAD /////////////////////////////////////////////

// Headers
#include <sched.h>
#include <stdexcept>
#include <stop_token>
#include <iostream>
#include <string>
#include <thread>
#include "pthread.h"
#include "sched.h"

// Function to assign a core to a thread
void pinThreadToCore(int core_number){
    // Initialize, and set to zero
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(core_number, &cpu_set);

    // Assign a thread, to a particular core using pthread
    int result = pthread_setaffinity_np(
        pthread_self(), 
        sizeof(cpu_set), 
        &cpu_set
    );

    // If the result is 0: success
    // Else, not a success
    if(result != 0){
        throw std::runtime_error("Thread assignment to core: " + std::to_string(core_number) + " is failed, with result " + std::to_string(result));
    }
}


// Function to verify the thread pinned or not
void verifyPinThread(int core_number){
    // cpu set
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);

    // get thread affinity
    pthread_getaffinity_np(
        pthread_self(),
        sizeof(cpu_set),
        &cpu_set  
    );

    // Clarify
    if(CPU_ISSET(core_number, &cpu_set)){
        throw std::runtime_error("Core assigned is not the expected core");
    }

    // To get the assigned core number
    int assigned_core = sched_getcpu();
    std::cout << "The assigned core is: " << assigned_core << std::endl;
}


void callTheFunctionToRun(){}

// Use the function to pin the thread
int main(){
    // Create a thread
    std::jthread trading_thread([](std::stop_token token){
       // Assign a core
        pinThreadToCore(3);
        while(!token.stop_requested()){
            callTheFunctionToRun();
        }
    });

    // To verify the core is assigned to the thread
    verifyPinThread(3);

    return 0;
}