cmake_minimum_required(VERSION 3.22)
project(cmockery)

set(CMAKE_CXX_STANDARD 14)




add_executable(run_test run_tests.cpp)
target_link_libraries(run_test PUBLIC cmockery)

add_executable(assert_module_test assert_module.cpp assert_module_test.cpp)
target_link_libraries(assert_module_test PUBLIC cmockery)

add_executable(assert_macro_test assert_macro.cpp assert_macro_test.cpp)
target_link_libraries(assert_macro_test PUBLIC cmockery)

add_executable(allocate_module_test allocate_module.cpp allocate_module_test.cpp)
target_link_libraries(allocate_module_test PUBLIC cmockery)

add_executable(calculator_test calculator.cpp calculator_test.cpp)
target_link_libraries(calculator_test PUBLIC cmockery)
