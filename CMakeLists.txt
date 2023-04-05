cmake_minimum_required(VERSION 3.22)
project(cmockery)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE "Debug")

#库文件的输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

enable_testing()

add_library(cmockery STATIC include/cmockery.h src/cmockery.cpp src/ctest_port.cpp include/ctest_port.h include/ctest_printers.h include/ctest_assert.h src/ctest_assert.cpp include/ctest_matcher.h)

add_subdirectory(example)



