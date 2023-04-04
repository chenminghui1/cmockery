cmake_minimum_required(VERSION 3.22)
project(cmockery)

set(CMAKE_CXX_STANDARD 14)



link_libraries(cmockery)

add_executable(run_test run_tests.cpp)

add_executable(assert_module_test assert_module.cpp assert_module_test.cpp)

add_executable(assert_macro_test assert_macro.cpp assert_macro_test.cpp)

add_executable(allocate_module_test allocate_module.cpp allocate_module_test.cpp)

add_executable(customer_database_test customer_database.cc customer_database_test.cc)

add_executable(calculator_test calculator_test.cpp calculator.cpp config.h)

add_executable(key_value_test key_value.cpp key_value_test.cpp)

add_executable(product_database_test product_database.cpp product_database_test.cpp)