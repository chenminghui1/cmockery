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
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include "../include/cmockery.h"



//让编译器知道需要在外部文件中查找函数
extern void increment_value(int * const value);
extern void decrement_value(int * const value);
/* This test case will fail, but the assert is caught by run_tests() and the
 * next test is executed. */
void increment_value_fail(void **state) {
    int* a = static_cast<int*>(new(1));
    increment_value(a);
}

// This test case succeeds since increment_value() asserts on the NULL pointer.
void increment_value_assert(void **state) {
    int* a = nullptr;
    expect_assert_failure(increment_value(a));
}

/* This test case fails since decrement_value() doesn't assert on a NULL
 * pointer. */
void decrement_value_fail(void **state) {
    int* a = static_cast<int*>(new(1));
    expect_assert_failure(decrement_value(a));
}

int main(int argc, char *argv[]) {
    const UnitTest tests[] = {
        unit_test(increment_value_fail),

        unit_test(increment_value_assert),
        unit_test(decrement_value_fail),
    };
    return run_tests(tests);
}
