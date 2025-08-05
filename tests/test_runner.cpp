#include "test_runner.h"

#include <iostream>
#include <vector>
#include <string>

// A struct to hold a test function and its name
struct TestCase {
    void (*function)();
    std::string name;
};

// A global vector to hold all the test cases
std::vector<TestCase> test_cases;

// Function to add a test to the list
void add_test(void (*function)(), const char* name) {
    test_cases.push_back({function, name});
}

// Forward declarations for test registration functions
void register_math_tests();

// The main test runner function
int main() {
    // Register all tests
    register_math_tests();

    int passed = 0;
    int failed = 0;

    for (const auto& test_case : test_cases) {
        std::cout << "Running test: " << test_case.name << "..." << std::endl;
        try {
            test_case.function();
            std::cout << "[PASS] " << test_case.name << std::endl;
            passed++;
        } catch (const TestFailure& e) {
            std::cerr << "[FAIL] " << test_case.name << ": " << e.what() << std::endl;
            failed++;
        } catch (const std::exception& e) {
            std::cerr << "[FAIL] " << test_case.name << " (uncaught exception): " << e.what() << std::endl;
            failed++;
        } catch (...) {
            std::cerr << "[FAIL] " << test_case.name << " (unknown exception)" << std::endl;
            failed++;
        }
    }

    std::cout << "\n--- Test Summary ---" << std::endl;
    std::cout << "Passed: " << passed << ", Failed: " << failed << std::endl;

    return failed > 0 ? 1 : 0;
}
