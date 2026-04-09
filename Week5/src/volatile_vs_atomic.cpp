/*
VOLATILE - It is a keyword used specifically to write on a speaific memory address i.e. want to access a physical memory address from the code.
And, we always want to fetch the address directly from memory, rather than storing in the cache registers - As, the memory address for that
variable or object can change anytime, during the execution of program and is not always static or the same.
And it also prevents compiler level optimizations and reordering. But, doesn't provide:
- CPU level reordering
- Atomicity
- Thread safety
- Happens beofre

volatile is for hardware. std::atomic is for threads. Never substitute one for the other.
*/