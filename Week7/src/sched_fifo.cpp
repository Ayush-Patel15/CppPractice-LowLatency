/*
By default, the linux uses the CFS (Completly Fair Scheduler) - it gives only a specific time duration to a process or a thread to complete
its execution i.e. it gives equally fair opportunities to all the threads to complete there execution. But, this switching in between the threads
is the exact problem for HFT's. As, switching - clears the L1, L2 cache, TLB table, and also resets the Branch History or Branch Prediction Table.

Linux provides real-time scheduling policies, to overwrite the properties of CFS
- SCHED_OTHER: The default CFS scheduler
- SCHED_FIFO: The First In First Out scheduler, with a priority preemption rule
- SCHED_RR: Round Robin based scheduler algorithm
- SCHED_DEADLINE: Deadline based scheduling, i.e. the job should be completed before this time

// HOW A SCHED_FIFO WORKS:
1. Runs until it voluntarily blocks (syscall, sleep, mutex wait)
2. Runs until it yields (sched_yield())
3. Runs until a HIGHER priority SCHED_FIFO thread becomes runnable
4. NEVER preempted by CFS threads regardless of their priority
5. NEVER preempted by the scheduler tick

Priorities for a SCHED_FIFO processes starts from 0 (lowest) to 99 (highest).

Priority 90: market data feed handler
             → must process incoming data immediately
             → highest priority — never miss a tick

Priority 80: order matching engine
             → processes data from feed handler
             → high priority but below feed

Priority 70: risk manager
             → checks limits before orders sent
             → must run before order router

Priority 60: order router
             → sends orders to exchange
             → runs after risk approval

Priority 50: position manager
             → updates positions after fills
             → important but not ultra-latency sensitive

Priority 10: logging thread
             → records everything
             → lowest real-time priority
             → can be starved briefly without consequence
*/


///////////////////// pinThreadToCore + setRealTimeScheduling ////////////////////////////////////
#include <cassert>
#include <sched.h>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <thread>
#include "pthread.h"
#include "sched.h"

// Method to pin a thread to a core
void pinThreadToCore(int core_number){
    // cpu set
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(core_number, &cpu_set);

    // Assign a thread
    int result = pthread_setaffinity_np(
        pthread_self(), 
        sizeof(cpu_set),
        &cpu_set
    );

    // Success or fail
    if(result != 0){
        throw std::runtime_error("Core assignment failed: " + std::to_string(result));
    }
}

// Method to set a real time fifo scheduling with priority
void setRealTimePriority(int priority){
    // Set priority
    struct sched_param param;
    param.sched_priority = priority;

    // Assign the priorit
    int result = pthread_setschedparam(
        pthread_self(),
        SCHED_FIFO,
        &param
    );

    // success or not
    if(result != 0){
        throw std::runtime_error("Priority setting failed as: " + std::to_string(result));
    }
}


// To set up a trading thread
void setUpATradingThread(int core_number, int priority){
    // Call the functions first
    pinThreadToCore(core_number);
    setRealTimePriority(priority);

    // Verfiy the setup
    cpu_set_t cpu_set;
    pthread_getaffinity_np(
        pthread_self(),
        sizeof(cpu_set),
        &cpu_set
    );
    assert(CPU_ISSET(core_number, &cpu_set));

    // Sched check or verification
    struct sched_param param;
    int policy;
    pthread_getschedparam(
        pthread_self(),
        &policy,
        &param
    );
    assert(policy == SCHED_FIFO);
    assert(param.sched_priority == priority);
}

void processMarketData() {};

// In the main function - assign
int main(){
    // Initialize a thread and assign
    std::jthread th([](std::stop_token token){
        // Set up the trading thread
        setUpATradingThread(3, 80);

        // Function calls
        while(!token.stop_requested()){
            processMarketData();
        }
    });

    // For requesting a stop on thread
    // th.request_stop();
    
    return 0;
}