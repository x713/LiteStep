#ifndef __TEST_RUNNER_H__
#define __TEST_RUNNER_H__

#include <iostream>
#include <string>
#include <stdexcept>

class TestFailure : public std::runtime_error {
public:
    TestFailure(const std::string& message) : std::runtime_error(message) {}
};

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        throw TestFailure("Assertion failed at " + std::string(__FILE__) + ":" + std::to_string(__LINE__)); \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        throw TestFailure("Assertion failed at " + std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
                          ". Expected: " + std::to_string(expected) + ", Actual: " + std::to_string(actual)); \
    }

void add_test(void (*function)(), const char* name);

#endif // __TEST_RUNNER_H__
