#pragma once

// Include files
#include <random>
#include <vector>

// The market data generator class
class MarketDataGenerator{
private:
    std::mt19937_64 rng{42};
    int64_t mid_price_ticks{1000};
    int next_order_id{0};

    // To store the active orders
    std::vector<int> active_orders;

public:
    // The event type class
    enum class EventType{
        ADD,
        CANCEL,
        AGGRESSIVE
    };

    // Event struct
    struct Event{
        EventType event_type;
        int order_id;
        int64_t price_tick;
        int64_t quantity;
        bool is_buy;
    };

    // To create the next event
    Event nextEvent(){
        std::uniform_int_distribution<int> type_dist(0, 9);
        int roll = type_dist(rng);
        // Based on the proportion
        if(roll < 7 || active_orders.empty()){
            return generateAdd();
        }
        else if(roll < 9){
            return generateCancel();
        }
        else{
            return generateAgressive();
        }
    }

private:
    // Method: to generate a Add event
    Event generateAdd(){
        // Get the uniform values
        std::uniform_int_distribution<int64_t> price_dist(-5, 5);
        std::uniform_int_distribution<int64_t> quantity_dist(1, 1000);
        std::uniform_int_distribution<int> buy_dist(0, 1);

        // Random values
        bool is_buy = buy_dist(rng);
        int64_t quantity = quantity_dist(rng);
        int64_t offset = price_dist(rng);
        int64_t price_tick = mid_price_ticks + offset + (is_buy ? -1 : 1);
        int order_id = next_order_id++;
        active_orders.push_back(order_id);

        return Event{EventType::ADD, order_id, price_tick, quantity, is_buy};
    }

    // Method: to generate a cancel event
    Event generateCancel(){
        std::uniform_int_distribution<size_t> idx_dist(0, active_orders.size() - 1);
        size_t index = idx_dist(rng);
        int order_id = active_orders[index];
        // Remove the order
        active_orders[index] = active_orders.back();
        active_orders.pop_back();        

        return Event{EventType::CANCEL, order_id, 0, 0, false};
    }

    // Method: to generate an aggressive order
    Event generateAgressive(){
        std::uniform_int_distribution<int64_t> qty_dist(1, 100);
        std::uniform_int_distribution<int> buy_dist(0, 1);
        // get valuess
        bool is_buy = buy_dist(rng);
        int64_t quantity = qty_dist(rng);
        int64_t price_tick = mid_price_ticks + (is_buy ? 10 : -10);
        int order_id = next_order_id++;

        return Event{EventType::AGGRESSIVE, order_id, price_tick, quantity, is_buy};
    }
};
