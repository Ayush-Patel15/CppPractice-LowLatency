/*
Part 2 — SIMD Price Update
You have an array of 1,000,000 float prices. Apply this update to every price:
new_price = old_price * multiplier + offset
Where multiplier = 1.0001f and offset = 0.5f.

Write two versions:
Version A — scalar loop
Version B — AVX intrinsics using _mm256_fmadd_ps

Measure both. Verify both produce identical results.
*/

// header files
#include <immintrin.h>
#include <iostream>
#include <chrono>
#include <vector>

// const iteration
const int ITERATIONS = 1'000'000;
const float MULTIPLIER = 1.0001f;
const float OFFSET = 0.5f;
// using namaspace
using Clock = std::chrono::high_resolution_clock;
using ns = std::chrono::nanoseconds;

// benchmark scalar loop
long long scalar_loop(float* input, float* scalar_output){
    auto start = Clock::now();
    for(int i=0; i<ITERATIONS; i++){
        scalar_output[i] = input[i] * MULTIPLIER + OFFSET;
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<ns>(end - start).count();
}

// benchmark avx loop
long long avx_loop(float* input, float* avx_output){
    // broadcast scalar constants to all 8 lanes of a 256-bit register
    __m256 multiplier = _mm256_set1_ps(MULTIPLIER);  // [1.0001, 1.0001, ..., 1.0001]
    __m256 offset     = _mm256_set1_ps(OFFSET);      // [0.5, 0.5, ..., 0.5]

    auto start = Clock::now();

    int i = 0;
    // process 8 floats per iteration
    for(; i <= ITERATIONS - 8; i += 8){
        // load 8 floats from input
        __m256 prices = _mm256_loadu_ps(input + i);

        // fmadd: prices * multiplier + offset  (fused multiply-add)
        __m256 result = _mm256_fmadd_ps(prices, multiplier, offset);

        // store 8 results to output
        _mm256_storeu_ps(avx_output + i, result);
    }

    // scalar cleanup for remaining elements
    for(; i < ITERATIONS; i++){
        avx_output[i] = input[i] * MULTIPLIER + OFFSET;
    }

    return std::chrono::duration_cast<ns>(Clock::now()-start).count();
}


int main(){
    // ── Setup ─────────────────────────────────────────────
    std::vector<float> input(ITERATIONS);
    std::vector<float> output_scalar(ITERATIONS);
    std::vector<float> output_avx(ITERATIONS);

    // fill input with prices between 50-200
    for(int i = 0; i < ITERATIONS; i++)
        input[i] = 50.0f + (float)(i % 150);

    // ── Warmup ────────────────────────────────────────────
    scalar_loop(input.data(), output_scalar.data());
    avx_loop(input.data(), output_avx.data());

    // ── Benchmark ─────────────────────────────────────────
    long long s1=0, s2=0, s3=0;
    long long a1=0, a2=0, a3=0;

    s1 = scalar_loop(input.data(), output_scalar.data());
    a1 = avx_loop(input.data(), output_avx.data());
    s2 = scalar_loop(input.data(), output_scalar.data());
    a2 = avx_loop(input.data(), output_avx.data());
    s3 = scalar_loop(input.data(), output_scalar.data());
    a3 = avx_loop(input.data(), output_avx.data());

    long long avg_scalar = (s1+s2+s3)/3;
    long long avg_avx    = (a1+a2+a3)/3;

    // ── Results ───────────────────────────────────────────
    std::cout << "\n=== SIMD Price Update (N=" << ITERATIONS << ") ===\n";
    std::cout << "Scalar: " << avg_scalar << " ns\n";
    std::cout << "AVX:    " << avg_avx    << " ns\n";
    std::cout << "Speedup: " << (double)avg_scalar / avg_avx << "x\n";
    return 0;
}