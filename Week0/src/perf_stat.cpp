/*
Create a matrix and iterate on it - row wise and column wise, to see the perf stat difference
*/

#include <iostream>

// Global constant
const int SIZE=4096;
int matrix[SIZE][SIZE];

// A row-wise addition function
long long rowAddition(){
    long long total = 0;
    for(int i=0; i<SIZE; i++){
        for(int j=0; j<SIZE; j++){
            total += matrix[i][j];
        }
    }
    return total;
}

// A column-wise addition function
long long colAddition(){
    long long total = 0;
    for(int j=0; j<SIZE; j++){
        for(int i=0; i<SIZE; i++){
            total += matrix[i][j];
        }
    }
    return total;
}

// Main function to see the difference
int main(int argc, char* argv[]){
    for(int i=0; i<SIZE; i++){
        for(int j=0; j<SIZE; j++){
            matrix[i][j] = i + j;
        }
    }
    // Based on the arg, process it
    if(argc > 1 && argv[1][0] == 'c'){
        std::cout << "Col Addition: " << colAddition() << std::endl;
    }
    else{
        std::cout << "Row Addition: " << rowAddition() << std::endl;
    }
}
