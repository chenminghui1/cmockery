#include <exception>
#include <cstring>
#include "../include/ctest_exception.h"
//
// Created by chenminghui1 on 23-4-8.
//
namespace ctest{
const char * memory_err::what() const throw() {
    return "Illegal memory access";
}

const char * memory_leak::what() const throw() {
    return " memory leak";
}

const char * assert_fail::what() const throw() {

    return err_reson;
}

const char * bad_expect_fail::what() const throw() {
    return "expect fail but success";
}

} //namespace ctest