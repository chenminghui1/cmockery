cmake_minimum_required(VERSION 3.22)
project(cmockery)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")
#设置编译选项，不做任何优化，打开所有警告，调试信息
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
#库文件的输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

enable_testing()

add_library(cmockery STATIC include/cmockery.h src/cmockery.cpp src/ctest_port.cpp include/ctest_port.h include/ctest_printers.h include/ctest_assert.h src/ctest_assert.cpp include/ctest_matcher.h src/ctest_excption.cpp include/ctest_exception.h src/ctest_log.cpp include/ctest_log.h src/ctest_log.cpp)

add_subdirectory(example)



