/*
AVERAGES

System A latencies (ns):
100, 101, 99, 100, 102, 98, 100, 101, 99, 100
Average: 100ns

System B latencies (ns):
50, 50, 50, 50, 50, 50, 50, 50, 50, 950
Average: 100ns

As the average of both the systems (A & B) is same 100 ns. But, System B has a latency spike of 950 ns, which is huge. Therefore, do not
rely on averages - for computing the latencies.

-----------------------------------------------------------------------------------------------------------------------------------

LATENCY HISTOGRAMS
We can create histogram of the latencies array or vector or whatever (must be sorted). Then, divide them in different bins of histogram,
and make a count. In this way also, we can get the latency of the overall system. There are two ways for this - 
- Linear difference: The difference between each bin is fixed, and we divide the bins. But, the worst case of tail latency can go
un noticed in this case, beacuse of large number of bins
Ex: 0-100ns, 100-200ns, 200-300ns...

- Log based: Good for latency distributions which span nanoseconds to milliseconds. Uses constant memory regardless of range. 
Loses precision at high values — bucket 20 covers 524us-1048us (500us range). Acceptable for tail analysis.
Ex: 0-1ns, 1-2ns, 2-4ns, 4-8ns, 8-16ns... 1ms-2ms...

-----------------------------------------------------------------------------------------------------------------------------------

HDR: High Dynamic Range Histogram
Precision:   1% relative error at all scales
Range:       1ns to hours in a single structure
Memory:      ~400KB fixed regardless of sample count
Operations:  O(1) record, O(1) percentile query

# install HDR histogram C++ library
sudo apt install libhdr-histogram-dev

#include <hdr/hdr_histogram.h>

hdr_histogram* hist = nullptr;
hdr_init(1, 3600000000000LL, 3, &hist);  // 1ns to 1hour, 3 sig figs

// record
hdr_record_value(hist, latency_ns);

// query
int64_t p50  = hdr_value_at_percentile(hist, 50.0);
int64_t p99  = hdr_value_at_percentile(hist, 99.0);
int64_t p999 = hdr_value_at_percentile(hist, 99.9);
*/
