//
// Created by oslab on 23-4-8.
//

#ifndef CMOCKERY_CTEST_EXCEPTION_H
#define CMOCKERY_CTEST_EXCEPTION_H
#include "string"

namespace ctest {
#undef delete

    class memory_err : public std::exception {
    public:
        const char *what() const throw();
    };

    class memory_leak : public std::exception {
    public:
        const char *what() const throw();
    };

    class assert_fail : public std::exception {
    public:
        ///TODO:禁止编译器自己产生构造函数
        assert_fail() = delete;

        assert_fail(char *reson) noexcept:
                err_reson{reson} {
        };

        const char *what() const throw();

    private:
        char *err_reson;
    };

    class bad_expect_fail : public std::exception {
    public:
        const char *what() const throw();
    };
}
#endif //CMOCKERY_CTEST_EXCEPTION_H
