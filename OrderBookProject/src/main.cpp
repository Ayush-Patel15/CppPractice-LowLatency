// headers
#include "LatencyHistogram.h"
#include "MarketDataGenrator.h"
#include "OrderBookSide.h"
#include "RdtscTimer.h"
#include <iostream>
#include <memory>
#include <vector>
#include <cstdlib>

// // For confirming zero heap allocations
// static int g_alloc_count;
// // Operator overload
// void* operator new(size_t size){
//     g_alloc_count++;
//     return malloc(size);
// }


// The main function
int main(){
    static const int WARMUP_ITERATIONS = 100'000;
    static const int MEASURE_ITERATIONS = 1'000'000;
    static const int TOTAL_EVENTS = WARMUP_ITERATIONS + MEASURE_ITERATIONS;
    static int rejected_order = 0;

    // Initialize everything
    LatencyHistogram add_hist, cancel_hist, aggresive_hist;
    MarketDataGenerator event_generator;
    auto order_book = std::make_unique<OrderBookSide>(900);
    // std::unique_ptr<OrderBookSide> order_book = std::make_unique<OrderBookSide>(900);
    RdtscTimer timer;

    // Set a callback
    order_book->setFillCallback([](const Fill&){});

    // Pre-generate the events
    std::vector<MarketDataGenerator::Event> all_events;
    all_events.reserve(TOTAL_EVENTS);
    for(int i=0; i<TOTAL_EVENTS; i++){
        all_events.push_back(event_generator.nextEvent());
    }

    // Warmup iterations
    for(int i=0; i<WARMUP_ITERATIONS; i++){
        auto& e = all_events[i];
        switch (e.event_type) {
            case MarketDataGenerator::EventType::ADD:
            case MarketDataGenerator::EventType::AGGRESSIVE:
                if(!order_book->addOrder(e.order_id, e.price_tick, e.quantity, e.is_buy)){
                    rejected_order++;
                };
                break;
            case MarketDataGenerator::EventType::CANCEL:
                order_book->cancelOrder(e.order_id);
                break;
        }
    }

    // g_alloc_count = 0;

    // Actual measurement iterations
    for(int i=WARMUP_ITERATIONS; i<TOTAL_EVENTS; i++){
        auto& e = all_events[i];
        timer.start();
        switch(e.event_type){
            case MarketDataGenerator::EventType::ADD:
            case MarketDataGenerator::EventType::AGGRESSIVE:{
                bool status = order_book->addOrder(e.order_id, e.price_tick, e.quantity, e.is_buy);
                if(!status) rejected_order++;
                break;
            }
            case MarketDataGenerator::EventType::CANCEL:
                order_book->cancelOrder(e.order_id);
                break;
        }
        timer.end();

        // For bucketing the timer
        switch (e.event_type) {
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

    // For heap allocation - before the print() function, so to skip the std::string heap allocation
    // std::cout << "\nCount of heap allocation, via new: " << g_alloc_count << "\n";

    // The final orderbook measurement
    std::cout << "\n======= OrderBook Measurment ===========" << "\n";
    add_hist.print("AddOrder");
    aggresive_hist.print("AggresiveOrder");
    cancel_hist.print("CancelOrder");

    // cout the rejected orders
    std::cout << "\nCount of rejected orders: " << rejected_order << "\n";
}