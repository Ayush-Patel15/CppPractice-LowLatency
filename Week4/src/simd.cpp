/*
SIMD - Single Instruction, Multiple Data. Today's modern generation registers support SIMD instructions i.e. is capable of performing
multiple instructions in a single register call.

MMX    (1997)  →  64-bit  registers  →  8×  8-bit  or 2× 32-bit
SSE    (1999)  →  128-bit registers  →  4× 32-bit float
SSE2   (2001)  →  128-bit registers  →  2× 64-bit double or 4× 32-bit int
SSE4.2 (2008)  →  128-bit registers  →  string/text operations added
AVX    (2011)  →  256-bit registers  →  8× 32-bit float or 4× 64-bit double
AVX2   (2013)  →  256-bit registers  →  integer operations added
AVX-512(2017)  →  512-bit registers  →  16× 32-bit float

SSE and AVX are the name or type of registers

// Scalar add operation in registers
ADD eax, ebx    → adds one integer to one integer → one result

// Vectorized add operation in registers
VADDPS ymm0, ymm1, ymm2    → adds 8 floats to 8 floats → 8 results

For vectorization, and say vectorized approach. There are 3 ways to achieve this in C++

- Using the -O3 -march=native (while compiling the code, it will automatically vectorize the operations)
- Call SIMD instructions directly through C++ wrapper functions called intrinsics. #include <immintrin.h>   // AVX intrinsics
- Or directly writing the assembly level code using the asm blocks

(-O3 -march=native is always the recommended way)
*/

/*
NOTE: Let's say there are 1003 elements to add. And we have an AVX2 supporting machine, that can perform 8 float instructions at once.

- So, first using the vectorized power, it will take the 1000 / 8 = 125 iterations to add the values.
- Then, will fall back to scalar addition for remaining 3 values. And, return a final addition.
- That 125 iterations will run independent of each other --> then, at last the vectorized result will be added.
*/