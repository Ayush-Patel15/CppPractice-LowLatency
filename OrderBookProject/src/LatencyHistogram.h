#pragma once

// include headers
#include <algorithm>
#include <climits>
#include <iostream>
#include <string>
#include <array>
#include <cmath>
#include <cstdint>

// The latency histogram buckets
class LatencyHistogram{
private:
    // attributes
    static constexpr int BUCKETS = 64;
    std::array<uint64_t, BUCKETS> buckets_arr{};
    uint64_t total_count{0};
    uint64_t total_sum{0};
    uint64_t min_val{UINT64_MAX};
    uint64_t max_val{0};

    // method: to get the bucker, to record for
    int indexToRecordFor(uint64_t latency_ns){
        if(latency_ns == 0) return 0;
        int index = (int)std::log2((double)latency_ns);
        return std::min(index, BUCKETS - 1);
    }

public:
    // Getter: To get the min value
    uint64_t min() const {
        return total_count ? min_val : 0;
    }

    // Getter: To get the average
    double average() const {
        return total_count ? (double)total_sum / total_count : 0.0;
    }

    // Method: To record the data point
    void record(uint64_t latency_ns){
        total_count++;
        total_sum += latency_ns;
        min_val = std::min(min_val, latency_ns);
        max_val = std::max(max_val, latency_ns);
        // Record the data
        int index = indexToRecordFor(latency_ns);
        buckets_arr[index]++;
    }

    // Method: To get the upper percentile bracket
    uint64_t percentile(double perc) const{
        if(total_count == 0) return 0;
        // Else, get the percentile
        uint64_t target = (uint64_t)((total_count * perc) / 100.0);
        uint64_t cumulative = 0;
        for(int i=0; i<BUCKETS; i++){
            cumulative += buckets_arr[i];
            if(cumulative >= target){
                return (uint64_t)1ULL << i;
            }
        }
        return max_val;
    }

    // Method: To print the data
    void print(const std::string& label = ""){
        if(!label.empty()){
            std::cout << "\n======= " << label << " =======\n";
        }
        std::cout << "Total samples: " << total_count << "\n";
        std::cout << "Average: " << average() << "\n";
        std::cout << "Min value: " << min() << "\n";
        std::cout << "Max value: " << max_val << "\n";
        std::cout << "p50: " << percentile(50) << "\n";
        std::cout << "p90: " << percentile(90) << "\n";
        std::cout << "p99: " << percentile(99) << "\n";
        std::cout << "p999: " << percentile(99.9) << "\n";

        // Iterate on each non-empty bucket
        std::cout << "\nHistogram buckets -->" << "\n";
        for(int i=0; i<BUCKETS; i++){
            if(buckets_arr[i] == 0) continue;
            uint64_t lower = (i == 0) ? 0 : 1ULL << (i-1);
            uint64_t upper = 1ULL << i;
            double pct = 100.0 * buckets_arr[i] / total_count;
            std::cout << lower << " lower - " << upper << " upper: " << pct << "\n";
        }
    }
};
