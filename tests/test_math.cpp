#include "test_runner.h"
#include "lsapi/MathEvaluate.h"
#include "lsapi/SettingsDefines.h"

#include <string>

void test_simple_addition() {
    SettingsMap context;
    std::wstring result;
    bool success = MathEvaluateString(context, L"1 + 1", result, StringSet());
    ASSERT_TRUE(success);
    ASSERT_EQ(std::wstring(L"2"), result);
}

void test_subtraction() {
    SettingsMap context;
    std::wstring result;
    bool success = MathEvaluateString(context, L"10 - 5", result, StringSet());
    ASSERT_TRUE(success);
    ASSERT_EQ(std::wstring(L"5"), result);
}

void test_multiplication() {
    SettingsMap context;
    std::wstring result;
    bool success = MathEvaluateString(context, L"2 * 3", result, StringSet());
    ASSERT_TRUE(success);
    ASSERT_EQ(std::wstring(L"6"), result);
}

void test_division() {
    SettingsMap context;
    std::wstring result;
    bool success = MathEvaluateString(context, L"10 / 2", result, StringSet());
    ASSERT_TRUE(success);
    ASSERT_EQ(std::wstring(L"5"), result);
}

void test_order_of_operations() {
    SettingsMap context;
    std::wstring result;
    bool success = MathEvaluateString(context, L"2 + 3 * 4", result, StringSet());
    ASSERT_TRUE(success);
    ASSERT_EQ(std::wstring(L"14"), result);
}

void test_negative_numbers() {
    SettingsMap context;
    std::wstring result;
    bool success = MathEvaluateString(context, L"-5 + 10", result, StringSet());
    ASSERT_TRUE(success);
    ASSERT_EQ(std::wstring(L"5"), result);
}

void test_floating_point() {
    SettingsMap context;
    std::wstring result;
    bool success = MathEvaluateString(context, L"1.5 + 2.5", result, StringSet());
    ASSERT_TRUE(success);
    ASSERT_EQ(std::wstring(L"4"), result);
}

void test_division_by_zero() {
    SettingsMap context;
    std::wstring result;
    bool success = MathEvaluateString(context, L"1 / 0", result, StringSet());
    ASSERT_TRUE(!success);
}

void test_syntax_error() {
    SettingsMap context;
    std::wstring result;
    bool success = MathEvaluateString(context, L"1 +", result, StringSet());
    ASSERT_TRUE(!success);
}

void register_math_tests() {
    add_test(test_simple_addition, "Test Simple Addition");
    add_test(test_subtraction, "Test Subtraction");
    add_test(test_multiplication, "Test Multiplication");
    add_test(test_division, "Test Division");
    add_test(test_order_of_operations, "Test Order of Operations");
    add_test(test_negative_numbers, "Test Negative Numbers");
    add_test(test_floating_point, "Test Floating Point");
    add_test(test_division_by_zero, "Test Division by Zero");
    add_test(test_syntax_error, "Test Syntax Error");
}
