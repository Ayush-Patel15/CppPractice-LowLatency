/*
CPU's are designed to store the data in natural alignment i.e. at addresses, where address % dataSize == 0

char   (1 byte)  →  can sit at any address       (1, 2, 3, 4, ...)
short  (2 bytes) →  prefers address % 2 == 0     (2, 4, 6, 8, ...)
int    (4 bytes) →  prefers address % 4 == 0     (4, 8, 12, 16, ...)
double (8 bytes) →  prefers address % 8 == 0     (8, 16, 24, 32, ...)

- Always prefer to store or list the datatype from biggest to the smallest size, while defining a struct - to reduce the extra padding
*/

#include <cstddef>
#include <iostream>

// A Bad struct with extra padding i.e. waste of memeory
struct Bad{
    char a;         // Occupies address 0 ==> 0 % 1 = 0 [0-1)
    int b;          // Occupies address 4 ==> 4 % 4 = 0 [4-8), Rest 1, 2, 3 are padding i.e. extra space added by the compiler
    char c;         // Occupies address 8 ==> 8 % 1 = 0 [8-9)
    double d;       // Occupies address 16 ==> 16 % 8 = 0 [16-24), Rest from 9 to 15 is the padding
};
// Total size of Struct bad is 24 bytes


// A good struct i.e. largest to smalles data size
struct Good{
    double d;       // Occupies from 0 to 8, ==> 0 % 8 = 0
    int b;          // Occupies from 8 to 12 ==> 8 % 4 = 0
    char a;         // Occupies the 12the address
    char c;         // Occupies the 13th address
};
// Total size of Struct Good is 16 bytes, the extra bytes are the tail padding added by the compiler


// For packet transfer, via the networks - we must reduce the data packet. However, for performance - adding padding works better
struct __attribute__((packed)) packedPacket{
    char a;
    int b;
    char c;
    double d;
};
// Total packet size is 14

int main(){
    // Get the size of both structs
    std::cout << "Size of struct Bad: " << sizeof(Bad) << std::endl;
    std::cout<< "Size of struct Good: " << sizeof(Good) << std::endl;
    std::cout << "Size of packed struct: " << sizeof(packedPacket) << std::endl;

    // Get the offsets
    std::cout << "Offsets of a: Bad - " << offsetof(Bad, a) << " Good - " << offsetof(Good, a) << " Packedpacket - " << offsetof(packedPacket, a) << std::endl;
    std::cout << "Offsets of b: Bad - " << offsetof(Bad, b) << " Good - " << offsetof(Good, b) << " Packedpacket - " << offsetof(packedPacket, b) << std::endl;
    std::cout << "Offsets of c: Bad - " << offsetof(Bad, c) << " Good - " << offsetof(Good, c) << " Packedpacket - " << offsetof(packedPacket, c) << std::endl;
    std::cout << "Offsets of d: Bad - " << offsetof(Bad, d) << " Good - " << offsetof(Good, d) << " Packedpacket - " << offsetof(packedPacket, d) << std::endl;
}
