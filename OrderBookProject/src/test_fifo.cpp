/*
Check the order of FIFO is followed, on the aggressive fill orders i.e. when the bid >= best ask index. All the order, that has the
ask price levels <= bid price --> should get a fill
*/

#include "OrderBookSide.h"
#include <iostream>
#include <cassert>
#include <vector>

// The test suite for 1 fill
int main(){
    // Construct the order book
    OrderBookSide order_book(900);
    // Vector to hold the filled events
    std::vector<Fill> fill_orders;

    // Set the callback function
    order_book.setFillCallback([&](const Fill& f){
        fill_orders.push_back(f);
    });

    // Create multiple ask orders
    order_book.addOrder(1, 1000, 50, false);
    order_book.addOrder(2, 1000, 50, false);
    order_book.addOrder(3, 1000, 50, false);
    order_book.addOrder(4, 1000, 50, false);
    // Create an aggresive fill, a bid order
    order_book.addOrder(5, 1010, 150, true);

    // Assert checks
    assert(fill_orders.size() == 3);
    assert(fill_orders[0].price_ticks == 1000);
    assert(fill_orders[0].existing_order_id == 1);
    assert(fill_orders[1].existing_order_id == 2);
    assert(fill_orders[2].existing_order_id == 3);
    assert(fill_orders[0].incoming_order_id == 5);
    assert(fill_orders[1].incoming_order_id == 5);
    assert(fill_orders[2].incoming_order_id == 5);
    assert(fill_orders[0].exec_qty == 50);
    assert(fill_orders[1].exec_qty == 50);
    assert(fill_orders[2].exec_qty == 50);

    std::cout << "Multi-order aggressive fill with FIFO passed\n";
}
