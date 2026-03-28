/*
To allocate memory of dynamic size - at runtime - in the stack frame of the program.
It is valid or exist in the program, till the function's lifetime or lifespan.
Not dependent on the scope, where it is defined - so, it don't follow the RAII principle and
doesn't have its destructor.
*/

// Header to include
#include <alloca.h>
#include <iostream>


// Function to initialize - dynamic stack memory
void allocate_dynamic_memory(int n){
    int* data = (int*)alloca(n * sizeof(int));
    std::cout << "Allocated stack memory for " << n << " elements, and total size is " << sizeof(data) << "\n"; 
    // sizeof() will return the size of the pointer, as data is the pointer to actual memory object
    return;
}


int main(){
    int n = 10;
    allocate_dynamic_memory(n);
    return 0;
}
