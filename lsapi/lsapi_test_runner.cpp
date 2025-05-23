#include "png_support.h" // This must declare RunPNGLoadTest()
#include <iostream>      // For std::cout

// main function for the test runner executable
int main() {
    std::cout << "------------------------------------" << std::endl;
    std::cout << "  LSAPI PNG Handling Test Runner    " << std::endl;
    std::cout << "------------------------------------" << std::endl;

    // Call the main test function from png_support.cpp
    bool test_result = RunPNGLoadTest();

    std::cout << "------------------------------------" << std::endl;
    if (test_result) {
        std::cout << "Overall Test Result: SUCCESS" << std::endl;
        return 0; // Indicate success to the OS
    } else {
        std::cout << "Overall Test Result: FAILURE" << std::endl;
        return 1; // Indicate failure to the OS
    }
}
