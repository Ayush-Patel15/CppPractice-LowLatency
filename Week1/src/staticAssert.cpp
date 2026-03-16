/*
To check certain conditions at the compile time, and fails the compilation - if the conditions are not met
As, it's a compile time check - there's no run-time overhead for them.
*/

#include <iostream>

// A order packet struct
struct alignas(64) Order{
    double price;
    long order_id;
    int quantity;
    char symbol[4];
    char status;
    bool is_buy;
};
// Total size of Order packet will be 64

int main(){
    // perform static-assert checks
    static_assert(sizeof(Order) == 64, "Size of Order struct is not 64 bytes");
    static_assert(offsetof(Order, price) == 0, "Order price is not at a offset of 0");
    static_assert(offsetof(Order, order_id) == 8, "Order's order_id is not at a offset of 8");
    static_assert(alignof(Order) == 64, "Align of Order struct is not 64");

    std::cout << "All asserts check completed, safe to run.." << std::endl;
}