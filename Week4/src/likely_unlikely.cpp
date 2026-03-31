/*
As, there's a dynamic branch predictor already present in th CPU. But, in the code - we can provide some suggestions to predict the branch
and let the conditin work in an optimized way. likeyly and unlikeyly doesn't guarantee to not create a branch - but helps in predeterming
the branch i.e. which is likely or unlikely to execute. And, for C++ 11 or so, there's an gcc compatible function of __builtin_expect(function, res).
res = 1, means likely branch and res = 0 unlikely branch. Main purpose of likely and unlikely - 

- Helps in prediction of branch, that is likely to be executed
- Keeps the likely branch in the same program stack space - so, that it can execute, without a jump to other memory address
*/

/*
if(order.is_valid()) [[likely]] {
    processOrder(order);      // hot path - stack path
} else [[unlikely]] {
    handleError(order);       // cold path - at a separate memory address
}

// __builtin_expect()
if(__builtin_expect(order.is_valid(), 1)){
    processOrder(order);      // 1 = expect true
}

if(__builtin_expect(order.is_valid(), 0)){
    handleError(order);       // 0 = expect false
}

// Using the macros
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

if(LIKELY(order.is_valid())){
    processOrder(order);
}

if(UNLIKELY(ptr == nullptr)){
    return nullptr;
}
*/

/*
// Where to Use Them in HFT Code - Null pointer checks — always unlikely:

void* raw = pool.allocate();
if(UNLIKELY(raw == nullptr)){
    // pool exhausted — exceptional case
    throw std::bad_alloc();
}
Order* o = new(raw) Order{...};

// Validation checks — invalid orders are rare:
if(LIKELY(order.quantity > 0 && order.price > 0.0)){
    matchOrder(order);
} else {
    UNLIKELY
    rejectOrder(order);
}
*/