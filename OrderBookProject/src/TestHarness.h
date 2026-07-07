#pragma once

// Include headers
#include <iostream>
#include <string>

// Global variables
inline int g_total_checks = 0;
inline int g_passed_checks = 0;
inline int g_failed_checks = 0;

// Define the check
#define CHECK(cond) \
    do{ \
        g_total_checks++; \
        if(!(cond)) {\
            g_failed_checks++; \
            std::cerr << "FAIL: " << #cond << "  [" << __FILE__ << ":" << __LINE__ << "]\n"; \
        } else{ \
            g_passed_checks++; \
        } \
    } while(0)


// The test
#define TEST(name) std::cout << "=========== " << name << " ================\n"; 

// The function
inline void printTestSummary(){
    std::cout << "\n=========================\n";
    std::cout << "Total:  " << g_total_checks  << "\n";
    std::cout << "Passed: " << g_passed_checks << "\n";
    std::cout << "Failed: " << g_failed_checks << "\n";
    std::cout << (g_failed_checks == 0 ? "ALL TESTS PASSED\n" : "SOME TESTS FAILED\n");
}
