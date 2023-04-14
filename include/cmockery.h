/*
 * Copyright 2008 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CMOCKERY_H_
#define CMOCKERY_H_
/*
 * These headers or their equivalents should be included prior to including
 * this header file.
 *
 * #include <stdarg.h>
 * #include <stddef.h>
 * #include <setjmp.h>
 *
 * This allows test applications to use custom definitions of C standard
 * library functions and types.
 */

#include "ctest_port.h"
#include "ctest_exception.h"
namespace ctest {
// For those who are used to __func__ from gcc.
#ifndef __func__
#define __func__ __FUNCTION__
#endif

#ifndef UNIT_TESTING
#define UNIT_TESTING 1
#endif //UNIT_TESTING

/* Largest integral type.  This type should be large enough to hold any
 * pointer or integer supported by the compiler. */
#ifndef LargestIntegralType
#define LargestIntegralType unsigned long long
#endif // LargestIntegralType

// Printf format used to display LargestIntegralType.
#ifndef LargestIntegralTypePrintfFormat
#define LargestIntegralTypePrintfFormat "%llx"
#endif // LargestIntegralTypePrintfFormat

// Perform an unsigned cast to LargestIntegralType.
#define cast_to_largest_integral_type(value) \
    ((LargestIntegralType)(value))
/* Smallest integral type capable of holding a pointer. */
#ifndef _UINTPTR_T
#define _UINTPTR_T
/* 在64位环境下，指针和long类型都是8字节 */
typedef unsigned long uintptr_t;
#endif//_UINTPTR_T

/* Perform an unsigned cast to uintptr_t. */
#define cast_to_pointer_integral_type(value) \
	((uintptr_t)(value))

/* Perform a cast of a pointer to uintmax_t */
#define cast_ptr_to_largest_integral_type(value) \
cast_to_largest_integral_type(cast_to_pointer_integral_type(value))

// Retrieves a return value for the current function.检索当前函数的返回值
#define mock() _mock(__func__, __FILE__, __LINE__)

/* 存储稍后由指定函数返回的值。count 参数返回 mock（） 应返回值的次数。
 * 如果 count 设置为 -1，则始终返回该值。
 * Stores a value to be returned by the specified function later.
 * The count parameter returns the number of times the value should be returned
 * by mock().  If count is set to -1 the value will always be returned.
 * TODO:函数对象替代
 */
#define will_return(function, value) \
    _will_return(#function, __FILE__, __LINE__, \
                    cast_to_largest_integral_type(value), 1)
#define will_return_count(function, value, count) \
    _will_return(#function, __FILE__, __LINE__, \
                    cast_to_largest_integral_type(value), count)

/* 添加自定义参数检查功能。如果事件参数为 NULL，则事件结构由
 * 此函数在内部分配。如果提供了事件参数，则必须在堆上分配该参数，并且调用方不需要将其解除分配。Add a custom parameter checking function.  If the event parameter is NULL
 * the event structure is allocated internally by this function.  If event
 * parameter is provided it must be allocated on the heap and doesn't need to
 * be deallocated by the caller.
 */
#define expect_check(function, parameter, check_function, check_data) \
    _expect_check(#function, #parameter, __FILE__, __LINE__, check_function, \
                    cast_to_largest_integral_type(check_data), NULL, 0)

/* Add an event to check a parameter, using check_expected(), against a set of
 * values. See will_return() for a description of the count parameter.
 */
#define expect_in_set(function, parameter, value_array) \
    expect_in_set_count(function, parameter, value_array, 1)
#define expect_in_set_count(function, parameter, value_array, count) \
    _expect_in_set(#function, #parameter, __FILE__, __LINE__, value_array, \
                    sizeof(value_array) / sizeof((value_array)[0]), count)
#define expect_not_in_set(function, parameter, value_array) \
    expect_not_in_set_count(function, parameter, value_array, 1)
#define expect_not_in_set_count(function, parameter, value_array, count) \
    _expect_not_in_set( \
        #function, #parameter, __FILE__, __LINE__, value_array, \
        sizeof(value_array) / sizeof((value_array)[0]), count)


/* Add an event to check a parameter, using check_expected(), against a
 * signed range.  Where range is minimum <= value <= maximum.
 * See will_return() for a description of the count parameter.
 */
#define expect_in_range(function, parameter, minimum, maximum) \
    expect_in_range_count(function, parameter, minimum, maximum, 1)
#define expect_in_range_count(function, parameter, minimum, maximum, count) \
    _expect_in_range(#function, #parameter, __FILE__, __LINE__, minimum, \
                        maximum, count)

/* Add an event to check a parameter, using check_expected(), against a
 * signed range.  Where range is value < minimum or value > maximum.
 * See will_return() for a description of the count parameter.
 */
#define expect_not_in_range(function, parameter, minimum, maximum) \
    expect_not_in_range_count(function, parameter, minimum, maximum, 1)
#define expect_not_in_range_count(function, parameter, minimum, maximum, \
                                    count) \
    _expect_not_in_range(#function, #parameter, __FILE__, __LINE__, \
                            minimum, maximum, count)

/* Add an event to check whether a parameter, using check_expected(), is or
 * isn't a value.  See will_return() for a description of the count parameter.
 */
#define expect_value(function, parameter, value) \
    expect_value_count(function, parameter, value, 1)
#define expect_value_count(function, parameter, value, count) \
    _expect_value(#function, #parameter, __FILE__, __LINE__, \
                    cast_to_largest_integral_type(value), count)
#define expect_not_value(function, parameter, value) \
    expect_not_value_count(function, parameter, value, 1)
#define expect_not_value_count(function, parameter, value, count) \
    _expect_not_value(#function, #parameter, __FILE__, __LINE__, \
                    cast_to_largest_integral_type(value), count)

/* Add an event to check whether a parameter, using check_expected(),
 * is or isn't a string.  See will_return() for a description of the count
 * parameter.
 */
#define expect_string(function, parameter, string) \
    expect_string_count(function, parameter, string, 1)
#define expect_string_count(function, parameter, string, count) \
    _expect_string(#function, #parameter, __FILE__, __LINE__, \
                    (const char*)(string), count)
#define expect_not_string(function, parameter, string) \
    expect_not_string_count(function, parameter, string, 1)
#define expect_not_string_count(function, parameter, string, count) \
    _expect_not_string(#function, #parameter, __FILE__, __LINE__, \
                        (const char*)(string), count)

/* Add an event to check whether a parameter, using check_expected() does or
 * doesn't match an area of memory.  See will_return() for a description of
 * the count parameter.
 */
#define expect_memory(function, parameter, memory, size) \
    expect_memory_count(function, parameter, memory, size, 1)
#define expect_memory_count(function, parameter, memory, size, count) \
    _expect_memory(#function, #parameter, __FILE__, __LINE__, \
                    (const void*)(memory), size, count)
#define expect_not_memory(function, parameter, memory, size) \
    expect_not_memory_count(function, parameter, memory, size, 1)
#define expect_not_memory_count(function, parameter, memory, size, count) \
    _expect_not_memory(#function, #parameter, __FILE__, __LINE__, \
                        (const void*)(memory), size, count)


/* Add an event to allow any value for a parameter checked using
 * check_expected().  See will_return() for a description of the count
 * parameter.
 */
#define expect_any(function, parameter) \
    expect_any_count(function, parameter, 1)
#define expect_any_count(function, parameter, count) \
    _expect_any(#function, #parameter, __FILE__, __LINE__, count)

/* Determine whether a function parameter is correct.  This ensures the next
 * value queued by one of the expect_*() macros matches the specified variable.
 */
#define check_expected(parameter) \
    _check_expected(__func__, #parameter, __FILE__, __LINE__, \
                    cast_to_largest_integral_type(parameter))

// 断言给定的表达式为真。参数可以被转化为unsigned long long类型即为真
#define assert_true(c) _assert_true(cast_to_largest_integral_type(c), #c, \
                                    __FILE__, __LINE__)
// 断言给定的表达式为假.
#define assert_false(c) _assert_false((cast_to_largest_integral_type(c)), #c, \
                                    __FILE__, __LINE__)
#define asssert_not_null(c) _assert_not_null(c, #c, __FILE__, __LINE__)
// Assert that the two given integers are equal, otherwise fail.
#define assert_int_equal(a, b) \
    _assert_int_equal(cast_to_largest_integral_type(a), \
                        cast_to_largest_integral_type(b), \
                        __FILE__, __LINE__)
// Assert that the two given integers are not equal, otherwise fail.
#define assert_int_not_equal(a, b) \
    _assert_int_not_equal(cast_to_largest_integral_type(a), \
                            cast_to_largest_integral_type(b), \
                            __FILE__, __LINE__)

// Assert that the two given strings are equal, otherwise fail.
#define assert_string_equal(a, b) \
    _assert_string_equal((const char*)(a), (const char*)(b), __FILE__, \
                            __LINE__)
// Assert that the two given strings are not equal, otherwise fail.
#define assert_string_not_equal(a, b) \
    _assert_string_not_equal((const char*)(a), (const char*)(b), __FILE__, \
                                __LINE__)

// Assert that the two given areas of memory are equal, otherwise fail.
#define assert_memory_equal(a, b, size) \
    _assert_memory_equal((const char*)(a), (const char*)(b), size, __FILE__, \
                            __LINE__)
// Assert that the two given areas of memory are not equal, otherwise fail.
#define assert_memory_not_equal(a, b, size) \
    _assert_memory_not_equal((const char*)(a), (const char*)(b), size, \
                                __FILE__, __LINE__)

// Assert that the specified value is >= minimum and <= maximum.
#define assert_in_range(value, minimum, maximum) \
    _assert_in_range( \
        cast_to_largest_integral_type(value), \
        cast_to_largest_integral_type(minimum), \
        cast_to_largest_integral_type(maximum), __FILE__, __LINE__)

// Assert that the specified value is < minumum or > maximum
#define assert_not_in_range(value, minimum, maximum) \
    _assert_not_in_range( \
        cast_to_largest_integral_type(value), \
        cast_to_largest_integral_type(minimum), \
        cast_to_largest_integral_type(maximum), __FILE__, __LINE__)

// Assert that the specified value is within a set.
#define assert_in_set(value, values, number_of_values) \
    _assert_in_set(value, values, number_of_values, __FILE__, __LINE__)
// Assert that the specified value is not within a set.
#define assert_not_in_set(value, values, number_of_values) \
    _assert_not_in_set(value, values, number_of_values, __FILE__, __LINE__)


// Forces the test to fail immediately and quit.
#define fail() _fail(__FILE__, __LINE__)

// Writes an error message and forces the test to fail immediately and quit
#define fail_msg(msg, ...) do { \
    print_error("ERROR: " msg "\n", ##__VA_ARGS__); \
    fail(); \
} while (0)

// Generic method to kick off testing启动测试的通用方法
#define run_test(f) _run_test(#f, f, NULL, UNIT_TEST_FUNCTION_TYPE_TEST, NULL)


///TODO:使用函数对象替换函数指针
// Initializes a UnitTest structure.
//unit_test是一个函数指针，它包含了函数名、函数指针和函数类型。
// 这个宏定义会将函数名、函数指针和函数类型作为参数传递给结构体tests，从而完成结构体的初始化。
#define unit_test(f) { #f, f, UNIT_TEST_FUNCTION_TYPE_TEST }
//prefix可以认为是suit，
#define unit_test_with_prefix(prefix, f) { #prefix#f, f, UNIT_TEST_FUNCTION_TYPE_TEST }
#define unit_test_setup(test, setup) \
    { #test "_" #setup, setup, UNIT_TEST_FUNCTION_TYPE_SETUP }
#define unit_test_teardown(test, teardown) \
    { #test "_" #teardown, teardown, UNIT_TEST_FUNCTION_TYPE_TEARDOWN }
// Used only for cmockery internal tests
#define unit_test_expect_failure(f) \
    { #f, f, UNIT_TEST_FUNCTION_TYPE_TEST_EXPECT_FAILURE}
/* Initialize an array of UnitTest structures with a setup function for a test
 * and a teardown function.  Either setup or teardown can be NULL.
 */
#define unit_test_setup_teardown(test, setup, teardown) \
    unit_test_setup(test, setup), \
    unit_test(test), \
    unit_test_teardown(test, teardown)

/*
 * Run tests specified by an array of UnitTest structures.  The following
 * example illustrates this macro's use with the unit_test macro.
 *
 * void Test0();
 * void Test1();
 *
 * int main(int argc, char* argv[]) {
 *     const UnitTest tests[] = {
 *         unit_test(Test0);
 *         unit_test(Test1);
 *         unit_test_with_prefix(SecondRun_, Test1);
 *     };
 *     return run_tests(tests);
 * }
 */
#define run_tests(tests) _run_tests(tests, sizeof(tests) / sizeof(tests)[0])

// Dynamic allocators
#define test_malloc(size) _test_malloc(size, __FILE__, __LINE__)
#define test_calloc(num, size) _test_calloc(num, size, __FILE__, __LINE__)
#define test_free(ptr) _test_free(ptr, __FILE__, __LINE__)

#define test_new(size)  _test_new(size,__FILE__, __LINE__)
#define test_delete(ptr)   _test_delete(ptr,__FILE__, __LINE__)

// Redirect malloc, calloc and free to the unit test allocators.
#if UNIT_TESTING
#define malloc test_malloc
#define calloc test_calloc
#define free test_free
#define new test_new
#define delete test_delete
#endif // UNIT_TESTING



///TODO:使用try{...} catch{...}替代setjmp
/*
 * 确保调用 mock_assert（）。如果调用 mock_assert（），则返回断言表达式字符串。
 * setjmp初始化完jump的上下文，返回0
 * For example:
 * #define assert mock_assert
 * void showmessage(const char *message) {
 *   assert(message);
 * }
 *
 * int main(int argc, const char* argv[]) {
 *   expect_assert_failure(show_message(NULL));
 *   printf("succeeded\n");
 *   return 0;
 * }
 */
//#define expect_assert_failure(function_call) \
//  { \
//    const int expression = setjmp(global_expect_assert_env); \
//    global_expecting_assert = 1; \
//    if (expression) { \
//      print_message("Expected assertion %s occurred\n", \
//                    *((const char**)&expression)); \
//      global_expecting_assert = 0; \
//    } else { \
//      function_call ; \
//      global_expecting_assert = 0; \
//      print_error("Expected assert in %s\n", #function_call); \
//      _fail(__FILE__, __LINE__); \
//    } \
//  }

#define expect_assert_failure(function_call) \
    {                                        \
        global_expecting_assert = 1;        \
        try{                              \
            function_call    ;                 \
            global_expecting_assert = 0;      \
            print_error("Expected assert in %s\n", #function_call); \
            throw bad_expect_fail();                                 \
            }                                \
            catch (bad_expect_fail &e) {          \
            throw;}\
        catch (...){                      \
            print_message("Expected assertion %s occurred\n" \
                    #function_call); \
            global_expecting_assert = 0;     \
            }\
    }
//函数对象
// Function prototype for setup, test and teardown functions.
typedef void (*UnitTestFunction)(void **state);
// Function that determines whether a function parameter value is correct.
typedef int (*CheckParameterValue)(const LargestIntegralType value,
                                    const LargestIntegralType check_value_data);

// Type of the unit test function.
enum UnitTestFunctionType {
    UNIT_TEST_FUNCTION_TYPE_TEST = 0,
    UNIT_TEST_FUNCTION_TYPE_SETUP,
    UNIT_TEST_FUNCTION_TYPE_TEARDOWN,
    UNIT_TEST_FUNCTION_TYPE_TEST_EXPECT_FAILURE,
} ;

/* Stores a unit test function with its name and type.
 * NOTE: Every setup function must be paired with a teardown function.  It's
 * possible to specify NULL function pointers.
 */
typedef struct UnitTest {
    const char* name;
    UnitTestFunction function;
    UnitTestFunctionType function_type;
} UnitTest;


// 在源代码中的位置
struct SourceLocation {
    const char* file;
    int line;
} ;

// Event that's called to check a parameter value.
typedef struct CheckParameterEvent {
    struct SourceLocation location;
    const char *parameter_name;
    CheckParameterValue check_value;
    LargestIntegralType check_value_data;
} CheckParameterEvent;

// Used by expect_assert_failure() and mock_assert().
extern int global_expecting_assert;
extern jmp_buf global_expect_assert_env;

// Retrieves a value for the given function, as set by "will_return".
//检索给定函数的值，由“will_return”设置。
LargestIntegralType _mock(const char * const function, const char* const file,
                          const int line);

void _expect_check(
    const char* const function, const char* const parameter,
    const char* const file, const int line,
    const CheckParameterValue check_function,
    const LargestIntegralType check_data, CheckParameterEvent * const event,
    const int count);

void _expect_in_set(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const LargestIntegralType values[],
    const size_t number_of_values, const int count);
void _expect_not_in_set(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const LargestIntegralType values[],
    const size_t number_of_values, const int count);

void _expect_in_range(
    const char* const function, const char* const parameter,
    const char* const file, const int line,
    const LargestIntegralType minimum,
    const LargestIntegralType maximum, const int count);
void _expect_not_in_range(
    const char* const function, const char* const parameter,
    const char* const file, const int line,
    const LargestIntegralType minimum,
    const LargestIntegralType maximum, const int count);

void _expect_value(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const LargestIntegralType value,
    const int count);
void _expect_not_value(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const LargestIntegralType value,
    const int count);

void _expect_string(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const char* string,
    const int count);
void _expect_not_string(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const char* string,
    const int count);

void _expect_memory(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const void* const memory,
    const size_t size, const int count);
void _expect_not_memory(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const void* const memory,
    const size_t size, const int count);

void _expect_any(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const int count);

void _check_expected(
    const char * const function_name, const char * const parameter_name,
    const char* file, const int line, const LargestIntegralType value);

// 可用于替换测试代码中的断言，以便与 check_assert（） 结合使用时，可以在不停止测试的情况下
// 确定断言条件是否失败。
// Can be used to replace assert in tested code so that in conjuction with
// check_assert() it's possible to determine whether an assert condition has
// failed without stopping a test.
void mock_assert(const int result, const char* const expression,
                 const char * const file, const int line);

void _will_return(const char * const function_name, const char * const file,
                  const int line, const LargestIntegralType value,
                  const int count);
void _assert_true(const LargestIntegralType result,
                  const char* const expression,
                  const char * const file, const int line);
void _assert_false(const LargestIntegralType result,
                  const char* const expression,
                  const char * const file, const int line);
void _assert_not_null(void *c, const char expression, const char * const file, const int line);
void _assert_int_equal(
    const LargestIntegralType a, const LargestIntegralType b,
    const char * const file, const int line);
void _assert_int_not_equal(
    const LargestIntegralType a, const LargestIntegralType b,
    const char * const file, const int line);
void _assert_string_equal(const char * const a, const char * const b,
                            const char * const file, const int line);
void _assert_string_not_equal(const char * const a, const char * const b,
                                const char *file, const int line);
void _assert_memory_equal(const void * const a, const void * const b,
                            const size_t size, const char* const file,
                            const int line);
void _assert_memory_not_equal(const void * const a, const void * const b,
                                const size_t size, const char* const file,
                                const int line);
void _assert_in_range(
    const LargestIntegralType value, const LargestIntegralType minimum,
    const LargestIntegralType maximum, const char* const file, const int line);
void _assert_not_in_range(
    const LargestIntegralType value, const LargestIntegralType minimum,
    const LargestIntegralType maximum, const char* const file, const int line);
void _assert_in_set(
    const LargestIntegralType value, const LargestIntegralType values[],
    const size_t number_of_values, const char* const file, const int line);
void _assert_not_in_set(
    const LargestIntegralType value, const LargestIntegralType values[],
    const size_t number_of_values, const char* const file, const int line);

void* _test_malloc(const size_t size, const char* file, const int line);
void* _test_calloc(const size_t number_of_elements, const size_t size,
                    const char* file, const int line);
void _test_free(void* const ptr, const char* file, const int line);

//////////
void* _test_new(const size_t size, const char* file, const int line);
void _test_delete(void* const ptr, const char* file, const int line);


void _fail(const char * const file, const int line);
void _run_test(const char *const function_name, const UnitTestFunction Function, void **const state,
               const UnitTestFunctionType function_type, const void *const heap_check_point, int &failed);
int _run_tests(const UnitTest * const tests, const size_t number_of_tests);

// Standard output and error print methods.
void print_message(const char* const format, ...);
void print_error(const char* const format, ...);
void vprint_message(const char* const format, va_list args);
void vprint_error(const char* const format, va_list args);
}
#endif // CMOCKERY_H_
