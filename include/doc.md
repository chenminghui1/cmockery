## header `cmockery.h`


## header 'ctest_port.h'
The following macros can be defined:

### Logging:

*   `CTEST_LOG_(severity)`
*   `CTEST_CHECK_(condition)`
*   Functions `LogToStderr()` and `FlushInfoLog()` have to be provided too.

### Threading:

*   `GTEST_HAS_NOTIFICATION_` - Enabled if Notification is already provided.
*   `GTEST_HAS_MUTEX_AND_THREAD_LOCAL_` - Enabled if `Mutex` and `ThreadLocal`
    are already provided. Must also provide `GTEST_DECLARE_STATIC_MUTEX_(mutex)`
    and `GTEST_DEFINE_STATIC_MUTEX_(mutex)`
*   `GTEST_EXCLUSIVE_LOCK_REQUIRED_(locks)`
*   `GTEST_LOCK_EXCLUDED_(locks)`

### Underlying library support features

*   `GTEST_HAS_CXXABI_H_`

### Exporting API symbols:

*   `GTEST_API_` - Specifier for exported symbols.

## Header `gtest-printers.h`

*   See documentation at `gtest/ctest-printers.h` for details on how to define a
    custom printer.

