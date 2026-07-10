// headers
#include "SPSCQueue.h"
#include "LatencyHistogram.h"
#include "MarketDataGenrator.h"
#include "OrderBookSide.h"
#include "RdtscTimer.h"
#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <cstdlib>
// For page faults benchmarking
#include <sys/resource.h>
#include <sys/time.h>

// // For confirming zero heap allocations
// static int g_alloc_count;
// // Operator overload
// void* operator new(size_t size){
//     g_alloc_count++;
//     return malloc(size);
// }

// Function: To get the minor page faults
long getMinorPageFaults(){
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_minflt;
}


// The main function
int main(){
    static const int WARMUP_ITERATIONS = 100'000;
    static const int MEASURE_ITERATIONS = 1'000'000;
    static const int TOTAL_EVENTS = WARMUP_ITERATIONS + MEASURE_ITERATIONS;
    static int rejected_order = 0;
    static int partially_filled_orders = 0;
    const size_t QUEUE_SIZE = 1024;

    // Initialize everything
    LatencyHistogram add_hist, cancel_hist, aggresive_hist;
    auto order_book = std::make_unique<OrderBookSide>(900);
    // std::unique_ptr<OrderBookSide> order_book = std::make_unique<OrderBookSide>(900);
    RdtscTimer timer;
    // Set a callback
    order_book->setFillCallback([](const Fill&){});
    // SPSPC queue
    SPSCQueue<MarketDataGenerator::Event, QUEUE_SIZE> queue;

    // The producer thread to produce
    std::jthread producer([&](){
        MarketDataGenerator event;
        for(int i=0; i<TOTAL_EVENTS; i++){
            MarketDataGenerator::Event e = event.nextEvent();
            while(!queue.push(e));
        };
    });

    // The consumer thread to consume
    std::jthread consumer([&](){
        MarketDataGenerator::Event e;
        long processed = 0;
        while(processed < TOTAL_EVENTS){
            while(!queue.pop(e)) continue;
            // start the time, for measurement only
            bool in_measurement = (processed >= WARMUP_ITERATIONS);
            if(in_measurement){
                timer.start();
            }
            // function call on event
            switch(e.event_type) {
                case MarketDataGenerator::EventType::ADD:
                case MarketDataGenerator::EventType::AGGRESSIVE:{
                    AddOrderState state = order_book->addOrder(e.order_id, e.price_tick, e.quantity, e.is_buy);
                    if(state == AddOrderState::REJECTED) rejected_order++;
                    else if(state == AddOrderState::PARTIALLY_FILLED) partially_filled_orders++;
                    break;
                }
                case MarketDataGenerator::EventType::CANCEL:
                    order_book->cancelOrder(e.order_id);
                    break;
            }
            // For latency measuremnt
            if(in_measurement){
                timer.end();
                switch(e.event_type) {
                    case MarketDataGenerator::EventType::ADD:
                        add_hist.record(timer.get_elapsed_ns());
                        break;
                    case MarketDataGenerator::EventType::AGGRESSIVE:
                        aggresive_hist.record(timer.get_elapsed_ns());
                        break;
                    case MarketDataGenerator::EventType::CANCEL:
                        cancel_hist.record(timer.get_elapsed_ns());
                        break;
                }
            }
            processed++;
        }
    });

    producer.join();
    consumer.join();

    // The final orderbook measurement
    std::cout << "\n======= OrderBook Measurment ===========" << "\n";
    add_hist.print("AddOrder");
    aggresive_hist.print("AggresiveOrder");
    cancel_hist.print("CancelOrder");

    // cout the orders
    std::cout << "\nCount of rejected orders: " << rejected_order << "\n";
    std::cout << "\nCount of partially filed orders: " << partially_filled_orders << "\n";
}