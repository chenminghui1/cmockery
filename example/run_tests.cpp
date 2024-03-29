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


#include <iostream>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include "../include/cmockery.h"

using namespace ctest;
// A test case that does nothing and succeeds.
void null_test_success(void **state) {
  std::cout << "hello testing!" << std::endl;
}

int main(int argc, char *argv[]) {
  const UnitTest tests[] = {
      unit_test(null_test_success),
      unit_test_with_prefix(someprefix_, null_test_success),
  };
  return run_tests(tests);
}
