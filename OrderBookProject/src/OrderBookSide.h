#pragma once

// The include headers
#include "PoolAllocator.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <new>
#include <utility>

/////////////////////////////// Order struct ////////////////////////////////////
struct Order{
    int order_id;
    int64_t price_ticks;
    int64_t quantity;
    bool is_buy;
    bool active;
    Order* next;
    Order* prev;
};


////////////////////////////////// PriceLevel Array ////////////////////////////
struct PriceLevel {
    int64_t total_quantity{0};
    int64_t order_count{0};
    bool active{false};
    Order* head{nullptr};
    Order* tail{nullptr};
};


/////////////////////////////// Fill Event struct /////////////////////////////
struct Fill{
    int incoming_order_id;
    int existing_order_id;
    int64_t price_ticks;
    int64_t exec_qty;
    bool incoming_is_buy;
};


////////////////////////////// Single orderbook side ////////////////////////////
class OrderBookSide{
private:
    // constants
    static const int MAX_LEVELS = 2048;
    static const int MAX_ORDERS = 50'000;

    // Price levels
    PriceLevel bid_levels[MAX_LEVELS];
    PriceLevel ask_levels[MAX_LEVELS];

    // best index, and price tick
    int best_bid_index{-1};
    int best_ask_index{MAX_LEVELS};
    int64_t base_price_ticks;

    // The order pool
    PoolAllocator order_pool;

    // Look up by id
    Order* order_lookup[MAX_ORDERS];

    // A function: for fill
    std::function<void(const Fill&)> on_fill;

public:
    // The constructor
    explicit OrderBookSide(int64_t base_price_ticks): base_price_ticks(base_price_ticks), order_pool(sizeof(Order), MAX_ORDERS){
        // Pre-initialize
        memset(bid_levels, 0, sizeof(bid_levels));
        memset(ask_levels, 0, sizeof(ask_levels));
        memset(order_lookup, 0, sizeof(order_lookup));
    }

    // Getter: index to price
    int64_t indexToPrice(int index) const {
        return base_price_ticks + index;
    }

    // Getter: price to index
    int priceToIndex(int64_t price_tick) const {
        return (int)(price_tick - base_price_ticks);
    }

    // Helper: Has bids or not
    bool hasBids() const {
        return best_bid_index != -1;
    }

    // Helper: has asks or not
    bool hasAsks() const {
        return best_ask_index != MAX_LEVELS;
    }

    // Method: Add an Order to a PriceLevel
    void addOrderToLevel(PriceLevel& level, Order* o){
        // To add at the tail, so FIFO is followed
        o->prev = level.tail;
        o->next = nullptr;
        // If tail exists, point the next to new order
        if(level.tail){
            level.tail->next = o;
        }
        // If it's the first order
        if(!level.head){
            level.head = o;
        }
        // action on all others
        level.tail = o;
        level.total_quantity += o->quantity;
        level.order_count++;
        level.active = true;
        return;
    }

    // Method: Remove an order, from the Pricelevel
    void removeOrderFromLevel(PriceLevel& level, Order* o){
        // Map the prev and next or current order
        if(o->next){
            o->next->prev = o->prev;
        }
        if(o->prev){
            o->prev->next = o->next;
        }
        // If the head order
        if(level.head == o){
            level.head = o->next;
        }
        // If the last order from the price level
        if(level.tail == 0){
            level.tail = o->prev;
        }
        // Remove the quantity
        level.total_quantity -= o->quantity;
        level.order_count--;
        if(level.order_count == 0){
            level.active = false;
            level.head = nullptr;
            level.tail = nullptr;
        }
    }

    // Method: To update the best index
    void updateBestIndex(int index, bool is_buy){
        if(is_buy){
            if(index > best_bid_index){
                best_bid_index = index;
            }
        }
        else{
            if(index < best_ask_index){
                best_ask_index = index;
            }
        }
    }

    // Method: Reset the best index
    void resetBestIndex(bool is_buy){
        if(is_buy){
            for(int i=best_bid_index-1; i>=0; i--){
                if(bid_levels[i].active){
                    best_bid_index = i;
                    return;
                }
            }
            best_bid_index = -1;
        }
        else{
            for(int i=best_ask_index+1; i<MAX_LEVELS; i++){
                if(ask_levels[i].active){
                    best_ask_index = i;
                    return;
                }
            }
            best_ask_index = MAX_LEVELS;
        }
    }

    // Method: To add the order
    void addOrder(int order_id, int64_t price_tick, int64_t quantity, bool is_buy){
        // First check for the order to match or fill
        int64_t remaining_qty = quantity;
        if(is_buy){
            while(remaining_qty > 0 && hasAsks() && price_tick >= indexToPrice(best_ask_index)){
                remaining_qty = matchAgainstLevel(ask_levels[best_ask_index], remaining_qty, order_id, is_buy);
                if(ask_levels[best_ask_index].order_count == 0){
                    // As need to reset the best ask index
                    resetBestIndex(false);
                }
            }
        }
        else{
            while(remaining_qty > 0 && hasBids() && price_tick <= indexToPrice(best_bid_index)){
                remaining_qty = matchAgainstLevel(bid_levels[best_bid_index], remaining_qty, order_id, is_buy);
                if(bid_levels[best_bid_index].order_count == 0){
                    // To reset the bid index
                    resetBestIndex(true);
                }
            }
        }
        // For purely adding
        if(remaining_qty > 0){
            // Take the memory
            void* raw = order_pool.allocate();
            Order* o = new(raw) Order{order_id, price_tick, remaining_qty, is_buy, true, nullptr, nullptr};
            // Get the price level - and add
            PriceLevel& level = is_buy ? bid_levels[priceToIndex(price_tick)] : ask_levels[priceToIndex(price_tick)];
            addOrderToLevel(level, o);
            // Assign to order look up
            order_lookup[order_id] = o;
            // Update the best index
            updateBestIndex(priceToIndex(price_tick), is_buy);
        }
        return;
    }

    // Method: To cancle a order
    void cancelOrder(int order_id){
        // Fetch the order
        Order* o = order_lookup[order_id];
        if(!o || !o->active) return;
        // Fetch the priceLevel and remove
        PriceLevel& level = o->is_buy ? bid_levels[priceToIndex(o->price_ticks)] : ask_levels[priceToIndex(o->price_ticks)];
        removeOrderFromLevel(level, o);
        // Remove from the order look up
        order_lookup[order_id] = nullptr;
        // Deallocate from pool
        o->~Order();
        order_pool.deallocate((void*)o);
        // re-build, if required
        if(level.active == false){
            resetBestIndex(o->is_buy);
        }
        return;
    }

    // Method: To match against the price level
    int64_t matchAgainstLevel(PriceLevel& level, int64_t incoming_qty, int order_id, bool incoming_is_buy){
        Order* curr = level.head;
        while (curr && incoming_qty > 0){
            int64_t fill_qty = std::min(incoming_qty, curr->quantity);
            // Call the Fill event, after a matching qty
            if(on_fill){
                try{
                    on_fill(
                        Fill{order_id, curr->order_id, curr->price_ticks, fill_qty, incoming_is_buy}
                    );
                }
                catch (...){
                    // Log and continue to next fill, doesn't disrupt the order book state or addOrder call stack
                }
                
            }
            // Decrement it
            incoming_qty -= fill_qty;
            level.total_quantity -= fill_qty;
            curr->quantity -= fill_qty;
            // Move to the next order
            Order* next = curr->next;
            if(curr->quantity == 0){
                removeOrderFromLevel(level, curr);
                order_lookup[curr->order_id] = nullptr;
                curr->~Order();
                order_pool.deallocate((void*)curr);
            }
            curr = next;
        }
        return incoming_qty;
    }

    // Method: A callback for the Fill event or fill std::function
    void setFillCallback(std::function<void(const Fill&)> cb){
        on_fill = std::move(cb);
        return;
    }
};
