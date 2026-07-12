# Low-Latency Order Book Engine

A price-time-priority matching engine built from scratch in C++20, designed around
the constraints of high-frequency trading systems - zero heap allocation on the hot
path, lock-free inter-thread communication, and cache-conscious data structures.
Every design decision in this project was validated through direct measurement
rather than assumption, including several optimizations that were tried, measured,
and reverted when the data contradicted the theory. It was developed and measured on a WSL2 system, not on a bare metal system.

---

## Performance Summary

Measured over 1,000,000 events (100,000 warmup iterations discarded), with
producer and consumer threads pinned to separate CPU cores, communicating via a
lock-free SPSC queue.

| Operation         | p50  | p90   | p99   | p999  |
|-------------------|------|-------|-------|-------|
| Add Order         | 24ns | 64ns  | 128ns | 256ns |
| Aggressive Match   | 32ns | 32ns  | 64ns  | 128ns |
| Cancel Order       | 96ns | 128ns | 256ns | 512ns |

Numbers are averaged across 5 independent full-scale runs. Tail latency (max
values) showed run-to-run variance ranging from ~100μs to a single ~8.9ms
outlier — consistent with known WSL2/Hyper-V scheduling jitter rather than a
structural issue in the matching engine itself (see "Environment Limitations"
below).

Full test suite: **38/38 checks passing** across 13 correctness tests, verified
clean under AddressSanitizer.

---

## Architecture

```
Market Data Generator ──▶ [SPSC Queue] ──▶ Matching Engine ──▶ Fill Callback
     (producer thread,                        (consumer thread,
      pinned to core 3)                         pinned to core 4)
```

**PoolAllocator** — `mmap`-backed fixed-size slot allocator. All memory is
reserved and pre-faulted once at startup; zero heap allocation occurs on the
hot path during trading. Verified with an `operator new` interception counter
showing 0 allocations during the full 1,000,000-event measurement window.

**OrderBookSide** — the core matching engine. Bid and ask price levels are
stored as fixed-size arrays indexed directly by price offset (O(1) access, no
tree traversal). Each price level holds an intrusive doubly-linked list of
orders (head = oldest, tail = newest), giving correct FIFO time-priority
matching with O(1) insertion and removal.

**OrderLookUpTable** — an open-addressing hash table mapping `order_id` to
`Order*`. Exists to solve a specific problem: order IDs increment
monotonically and unboundedly over the life of a trading session, but a
direct-indexed array sized to the maximum possible ID would require
unbounded memory. The hash table decouples lookup capacity from ID range,
at a measured, accepted latency cost (see below).

**SPSCQueue** — a lock-free single-producer/single-consumer ring buffer
using `std::atomic` with `release`/`acquire` memory ordering. No mutex, no
kernel involvement in the steady state. Head and tail indices are
cache-line-separated (`alignas(64)`) to prevent false sharing between the
producer and consumer threads.

**MarketDataGenerator** — produces a reproducible (fixed-seed) synthetic
stream of Add, Cancel, and Aggressive order events for benchmarking, in
place of a live exchange feed.

---

## Key Design Decisions

**Array-indexed price levels instead of `std::map`.** A balanced tree gives
O(log n) access but requires heap allocation per node and produces
cache-hostile pointer chasing. Since price levels for any single instrument
occupy a narrow, bounded range around the current market price, direct array
indexing gives true O(1) access with fully sequential, cache-friendly memory
layout.

**Intrusive linked lists instead of `std::vector` per price level.** Orders
at the same price level need O(1) insertion (append) and O(1) removal
(unlink) without heap allocation. Embedding `next`/`prev` pointers directly
in the `Order` struct achieves this — the linked list requires no separate
allocation, since the "nodes" are the orders themselves.

**Explicit rejection over silent failure.** `addOrder` returns a tri-state
`AddOrderState` (`COMPLETED`, `PARTIALLY_FILLED`, `REJECTED`) rather than a
boolean, so that a caller can distinguish "nothing happened" from "some
matching occurred but the remainder couldn't rest in the book" — a real
correctness gap found and fixed during development (see below).

**rdtsc over `std::chrono` for measurement.** `std::chrono::high_resolution_clock`
costs ~20-30ns per call via its underlying `clock_gettime` syscall — for
operations measured in tens of nanoseconds, the timer itself becomes the
dominant cost. Switching to the `RDTSC` hardware cycle counter (~1ns
overhead) reduced measured p50 latency by roughly 16x purely by removing
measurement noise, with no change to the underlying code.

---

## What I Measured and Reverted

Not every theoretically-motivated optimization improved real performance.
Documenting the failures alongside the successes:

**Cache-line alignment (`alignas(64)` on `Order` and `PriceLevel`).**
Theoretically sound — eliminates cache-line straddling for frequently
accessed structs. Measured result: every latency metric got *worse* across
three consecutive full-scale runs (CancelOrder p50 doubled from 64ns to
128ns; AddOrder max latency increased 3-9x). The likely cause: alignment
padding increased total memory footprint by 30-60% across the hot arrays,
increasing overall cache pressure by more than the per-object straddling
cost it eliminated. **Reverted** to natural (unaligned) struct layout.

**Open-addressing hash table vs. flat direct-indexed array for order
lookup.** The flat array (`Order* table[MAX_ORDER_ID]`) gives true O(1)
access with zero computation, but requires sizing to the maximum possible
order ID — 12MB+ for a realistic ID range, and still fundamentally unsafe
against IDs exceeding that bound. The hash table decouples capacity from ID
range safely, but measured a consistent ~4x typical-case latency cost
(16ns → 64ns) versus the flat array, from hashing and probing overhead —
a cost that persisted regardless of table size (tested at 2^18, 2^20
slots) and was not primarily a load-factor effect. **Kept** — the absolute
cost (tens of nanoseconds) was judged an acceptable price for eliminating
a real, unbounded-memory-growth correctness risk, but the tradeoff is
explicit and measured, not assumed.

---

## Threading Model

Market data generation and order matching run on separate, pinned threads
connected by the SPSC queue, decoupling I/O/generation latency from the
matching engine's critical path — matching never stalls waiting on the
producer.

Measured cost of this architecture:
- **Single-threaded baseline:** AddOrder p50 = 16ns
- **Multi-threaded, unpinned:** AddOrder p50 = 32ns, occasional multi-millisecond
  tail spikes from OS thread migration
- **Multi-threaded, pinned to separate cores:** AddOrder p50 = 32ns (unchanged),
  tail spikes reduced by roughly 10x (migration-induced spikes eliminated)

CPU pinning eliminates OS-scheduling-induced tail latency but cannot remove
the baseline ~2x typical-case cost of genuine cross-core synchronization
(the atomic operations underlying the SPSC queue itself). Both effects were
isolated and measured independently rather than conflated.

---

## Bugs Found and Fixed During Development

Several non-trivial correctness bugs were found through rigorous testing
(gdb, AddressSanitizer, and a purpose-built regression test suite) rather
than assumed away:

- A stale `order_lookup` entry after a fully-filled order was never cleared,
  causing a later cancel on the reused ID to corrupt a live, unrelated order.
- An intrusive linked-list insertion bug caused LIFO (most-recent-first)
  matching instead of correct FIFO (arrival-order) time priority — found via
  a targeted multi-order sweep test, fixed by adding proper tail-pointer
  tracking.
- A one-character typo (`level.tail == 0` instead of `level.tail == o`)
  silently disabled tail-pointer updates entirely, causing dangling-pointer
  memory corruption under sustained order churn — found via AddressSanitizer
  after a full-scale benchmark hang.
- `addOrder` returning a simple boolean conflated "fully rejected, nothing
  happened" with "partial match occurred, then the remainder couldn't rest" —
  fixed with a three-state result enum.

---

## Environment Limitations

Development and all benchmarking were done on WSL2 (Ubuntu on Windows/Hyper-V),
which introduces measurement constraints worth being explicit about:

- Hardware performance counters (`perf stat`) are partially unavailable;
  `cachegrind` (software cache simulation) and `getrusage`-based page-fault
  counting were used as reliable alternatives.
- `isolcpus`, `nohz_full`, and IRQ affinity — kernel-level core isolation
  techniques — are not configurable inside WSL2's virtualized kernel.
- Rare multi-millisecond tail latency spikes observed in benchmarks are
  consistent with Hyper-V scheduling jitter rather than application-level
  behavior; bare-metal Linux with full core isolation would be expected to
  produce meaningfully tighter tail latency.

---

## Building and Running

```bash
# Compile the order book benchmark (threaded, pinned)
g++ -std=c++20 -O2 -pthread -o outputs/main_spsc src/main_spsc.cpp
./outputs/main_spsc

# Compile and run the correctness test suite
g++ -std=c++20 -O0 -g -o outputs/test_suite src/test_suite.cpp
./outputs/test_suite

# Optional: verify memory safety with AddressSanitizer
g++ -std=c++20 -g -O0 -fsanitize=address -o outputs/test_suite_asan src/test_suite.cpp
./outputs/test_suite_asan
```

### Configuration

| Constant                | Value      | Purpose                                    |
|--------------------------|------------|---------------------------------------------|
| `MAX_LEVELS`              | 2048       | Price levels per side (bid/ask)              |
| `MAX_SLOTS`               | 1,048,576  | Order lookup hash table capacity              |
| `MAX_CONCURRENT_ORDERS`   | 500,000    | Pool allocator capacity                       |
| `WARMUP_ITERATIONS`       | 100,000    | Discarded before measurement begins            |
| `MEASURE_ITERATIONS`      | 1,000,000  | Events measured per benchmark run             |

---

## What This Project Does Not Cover

Scoped deliberately for a learning project, not a production system:

- No network layer or real exchange feed parsing (ITCH/OUCH/FIX)
- No periodic hash table compaction (tombstones accumulate indefinitely;
  acceptable for benchmark-length runs, a real gap for multi-day uptime)
- Single-instrument order book only, no cross-instrument risk management
- No persistence, recovery, or crash-safety guarantees