#include "TestHarness.h"
#include "OrderBookSide.h"
#include <vector>

// ── Test 1: Basic resting order ──────────────────────────
void test_basicRestingOrder(){
    TEST("Basic resting order");
    OrderBookSide book(900);

    book.addOrder(1, 1000, 100, true);   // resting bid, nothing to match against

    CHECK(book.hasBids() == true);
    CHECK(book.hasAsks() == false);
}

// ── Test 2: Basic cancel ──────────────────────────────────
void test_basicCancel(){
    TEST("Basic cancel");
    OrderBookSide book(900);

    book.addOrder(1, 1000, 100, true);
    CHECK(book.hasBids() == true);

    book.cancelOrder(1);
    CHECK(book.hasBids() == false);   // level should be empty, best_bid_index reset to -1
}

// ── Test 3: Time priority / FIFO ─────────────────────────
void test_timePriority(){
    TEST("Time priority (FIFO) — verifies head/tail fix");
    std::vector<Fill> fills;
    OrderBookSide book(900);
    book.setFillCallback([&](const Fill& f){ fills.push_back(f); });

    book.addOrder(1, 1000, 50, false);   // existing ask #1 — arrives first
    book.addOrder(2, 1000, 50, false);   // existing ask #2 — arrives second
    book.addOrder(3, 1000, 50, false);   // existing ask #3 — arrives third

    book.addOrder(4, 1000, 150, true);   // aggressive buy sweeps all three

    CHECK(fills.size() == 3);
    if(fills.size() == 3){
        CHECK(fills[0].existing_order_id == 1);   // must fill in ARRIVAL order
        CHECK(fills[1].existing_order_id == 2);
        CHECK(fills[2].existing_order_id == 3);
    }
}

// ── Test 4: Partial fill ─────────────────────────────────
void test_partialFill(){
    TEST("Partial fill — existing order stays in book with reduced qty");
    std::vector<Fill> fills;
    OrderBookSide book(900);
    book.setFillCallback([&](const Fill& f){ fills.push_back(f); });

    book.addOrder(1, 1000, 100, false);  // existing ask: 100 shares
    book.addOrder(2, 1000, 40,  true);   // aggressive buy: only 40 — partial

    CHECK(fills.size() == 1);
    if(fills.size() == 1){
        CHECK(fills[0].exec_qty == 40);
    }

    // existing order should still be resting with 60 remaining
    CHECK(book.hasAsks() == true);

    // cancel it and confirm no crash — proves the order is still alive & valid
    book.cancelOrder(1);
    CHECK(book.hasAsks() == false);
}

// ── Test 5: Full fill removes order completely ───────────
void test_fullFill(){
    TEST("Full fill — existing order fully consumed and removed");
    std::vector<Fill> fills;
    OrderBookSide book(900);
    book.setFillCallback([&](const Fill& f){ fills.push_back(f); });

    book.addOrder(1, 1000, 50, false);
    book.addOrder(2, 1000, 50, true);   // exact match, fully consumes order 1

    CHECK(fills.size() == 1);
    CHECK(book.hasAsks() == false);   // level should be empty now

    // cancelling the already-filled order must be a safe no-op, not a crash
    book.cancelOrder(1);
    CHECK(book.hasAsks() == false);   // still false — nothing to corrupt
}

// ── Test 6: Multi-level sweep ─────────────────────────────
void test_multiLevelSweep(){
    TEST("Multi-level sweep — aggressive order consumes multiple price levels");
    std::vector<Fill> fills;
    OrderBookSide book(900);
    book.setFillCallback([&](const Fill& f){ fills.push_back(f); });

    book.addOrder(1, 1000, 50, false);   // ask @ 1000
    book.addOrder(2, 1001, 50, false);   // ask @ 1001
    book.addOrder(3, 1002, 50, false);   // ask @ 1002

    // aggressive buy willing to pay up to 1002, wants 150 total
    book.addOrder(4, 1002, 150, true);

    CHECK(fills.size() == 3);
    if(fills.size() == 3){
        // each fill executes at the EXISTING order's price, not the aggressor's
        CHECK(fills[0].price_ticks == 1000);
        CHECK(fills[1].price_ticks == 1001);
        CHECK(fills[2].price_ticks == 1002);
    }
    CHECK(book.hasAsks() == false);   // fully swept
}

// ── Test 7: Double-cancel safety ─────────────────────────
void test_doubleCancel(){
    TEST("Double-cancel safety");
    OrderBookSide book(900);

    book.addOrder(1, 1000, 100, true);
    book.cancelOrder(1);
    book.cancelOrder(1);   // second cancel — must be a no-op, not a crash/corruption

    CHECK(book.hasBids() == false);
}

// ── Test 8: Cancel unknown order ─────────────────────────
void test_cancelUnknownOrder(){
    TEST("Cancel never-added order ID");
    OrderBookSide book(900);

    book.cancelOrder(9999);   // never added — must be safe no-op
    CHECK(book.hasBids() == false);
    CHECK(book.hasAsks() == false);
}

// ── Test 9: Crossed book invariant ───────────────────────
void test_neverCrossed(){
    TEST("Book never left in crossed state");
    OrderBookSide book(900);

    book.addOrder(1, 998,  100, true);    // resting bid
    book.addOrder(2, 1002, 100, false);   // resting ask, no match (998 < 1002)

    CHECK(book.hasBids() && book.hasAsks());
    // NOTE: requires OrderBookSide to expose best bid/ask price for direct comparison.
    // If not yet exposed, this test is a placeholder for when those getters exist.
}

// ── Test 10: Pool allocation balance ─────────────────────
void test_poolBalance(){
    TEST("Pool allocate/deallocate balance across mixed operations");
    OrderBookSide book(900);

    for(int i = 1; i <= 20; i++){
        book.addOrder(i, 1000 + (i % 5), 10, (i % 2 == 0));
    }
    for(int i = 1; i <= 20; i++){
        book.cancelOrder(i);
    }

    CHECK(book.hasBids() == false);
    CHECK(book.hasAsks() == false);
    // if the pool ever leaked or double-freed, this pattern of add-then-cancel-all
    // followed by re-adding the same range should behave identically on a second pass
    for(int i = 1; i <= 20; i++){
        book.addOrder(i, 1000 + (i % 5), 10, (i % 2 == 0));
    }
    CHECK(book.hasBids() || book.hasAsks());
}

// ── Test 11: Price Overflow, returns false ─────────────────────
void test_priceOverflowRejected(){
    TEST("Price outside representable range is rejected, not corrupting memory");
    OrderBookSide book(900);   // valid range: [900, 900+2048) = [900, 2948)

    bool result_valid   = book.addOrder(1, 1000, 100, true);   // in range
    bool result_too_low = book.addOrder(2, 500,  100, true);   // below base — invalid
    bool result_too_high= book.addOrder(3, 5000, 100, true);   // way above range — invalid

    CHECK(result_valid   == true);
    CHECK(result_too_low == false);
    CHECK(result_too_high== false);

    // book state should be unaffected by the rejected orders
    CHECK(book.hasBids() == true);   // only order 1 should be resting
}

// --- Aggressive sell test suite ---
void test_aggressiveSellMatchesFIFO(){
    TEST("Aggressive sell matches against bids in FIFO order");
    std::vector<Fill> fills;
    OrderBookSide book(900);
    book.setFillCallback([&](const Fill& f){ fills.push_back(f); });

    // 3 existing bids at the same price, arriving in order
    book.addOrder(1, 1000, 50, true);
    book.addOrder(2, 1000, 50, true);
    book.addOrder(3, 1000, 50, true);

    // aggressive sell — willing to accept as low as 990, sells 150 total
    bool ok = book.addOrder(4, 990, 150, false);

    CHECK(ok == true);
    CHECK(fills.size() == 3);
    if(fills.size() == 3){
        CHECK(fills[0].existing_order_id == 1);
        CHECK(fills[1].existing_order_id == 2);
        CHECK(fills[2].existing_order_id == 3);
        CHECK(fills[0].price_ticks == 1000);   // executes at EXISTING bid price, not 990
    }
    CHECK(book.hasBids() == false);
}


// ── Runner ─────────────────────────────────────────────────
int main(){
    test_basicRestingOrder();
    test_basicCancel();
    test_timePriority();
    test_partialFill();
    test_fullFill();
    test_multiLevelSweep();
    test_doubleCancel();
    test_cancelUnknownOrder();
    test_neverCrossed();
    test_poolBalance();
    test_priceOverflowRejected();
    test_aggressiveSellMatchesFIFO();

    printTestSummary();
    return g_failed_checks == 0 ? 0 : 1;   // non-zero exit code on failure — useful for CI later
}