//
// Created by oslab on 23-4-5.
//

#ifndef CMOCKERY_CTEST_ASSERT_H
#define CMOCKERY_CTEST_ASSERT_H

#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include "ctest_port.h"
class CTEST_API_ AssertionResult {
public:
  // Copy constructor.
  // Used in EXPECT_TRUE/FALSE(assertion_result).
  AssertionResult(const AssertionResult& other);


};


#endif // CMOCKERY_CTEST_ASSERT_H
