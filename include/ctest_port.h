//
// Created by chenminghui on 23-4-1.

// Logging:
//   GTEST_LOG_()   - logs messages at the specified severity level.
//   LogToStderr()  - directs all log messages to stderr.
//   FlushInfoLog() - flushes informational log messages.
//
// Stdout and stderr capturing:
//   CaptureStdout()     - starts capturing stdout.
//   GetCapturedStdout() - stops capturing stdout and returns the captured
//                         string.
//   CaptureStderr()     - starts capturing stderr.
//   GetCapturedStderr() - stops capturing stderr and returns the captured
//                         string.
//
// Integer types:
//   TypeWithSize   - maps an integer to a int type.
//   TimeInMillis   - integers of known sizes.
//   BiggestInt     - the biggest signed integer type.
//
// Command-line utilities:
//   GetInjectableArgvs() - returns the command line as a vector of strings.
//
// Environment variable utilities:
//   GetEnv()             - gets the value of an environment variable.
//   BoolFromGTestEnv()   - parses a bool environment variable.
//   Int32FromGTestEnv()  - parses an int32_t environment variable.
//   StringFromGTestEnv() - parses a string environment variable.
//
// Deprecation warnings:
//   GTEST_INTERNAL_DEPRECATED(message) - attribute marking a function as
//                                        deprecated; calling a marked function
//                                        should generate a compiler warning

#ifndef CMOCKERY_CTEST_PORT_H
#define CMOCKERY_CTEST_PORT_H
///TODO: 实现Logging:功能和Threading:
#include <cctype>   // for isspace, etc
#include <cstddef>  // for ptrdiff_t
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cerrno>
// #include <condition_variable>  // Guarded by GTEST_IS_THREADSAFE below
#include <cstdint>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <ostream>
#include <string>
// #include <mutex>  // Guarded by GTEST_IS_THREADSAFE below
#include <tuple>
#include <type_traits>
#include <vector>

#define CTEST_API_

// Determines the version of gcc that is used to compile this.
#ifdef __GNUC__
// 40302 means version 4.3.2.
#define GTEST_GCC_VER_ \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif  // __GNUC__

//正则表达式匹配
#include <regex.h>
#define GTEST_USES_POSIX_RE 1

#ifndef GTEST_HAS_EXCEPTIONS
// The user didn't tell us whether exceptions are enabled, so we need
// to figure it out.
#if defined(__BORLANDC__)
#ifndef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 1
#endif  // _HAS_EXCEPTIONS
#define GTEST_HAS_EXCEPTIONS _HAS_EXCEPTIONS
#endif //defined(__BORLANDC__)
#endif//GTEST_HAS_EXCEPTIONS


#if GTEST_HAS_PTHREAD
// ctest-port.h guarantees to #include <pthread.h> when GTEST_HAS_PTHREAD is
// true.
#include <pthread.h>  // NOLINT
// For timespec and nanosleep, used below.
#include <time.h>  // NOLINT
#endif


class Message;

// Legacy imports for backwards compatibility.
// New code should use std:: names directly.
using std::get;
using std::make_tuple;
using std::tuple;
using std::tuple_element;
using std::tuple_size;

// A secret type that Google Test users don't know about.  It has no
// definition on purpose.  Therefore it's impossible to create a
// Secret object, which is what we want.
class Secret;

// A helper for suppressing warnings on constant condition.  It just
// returns 'condition'.
CTEST_API_ bool IsTrue(bool condition);

// A simple C++ wrapper for <regex.h>.  It uses the POSIX Extended
// Regular Expression syntax.
class CTEST_API_ RE {

public:
  // A copy constructor is required by the Standard to initialize object
  // references from r-values.
  RE(const RE &other) { Init(other.pattern()); }

  // Constructs an RE from a string.
  RE(const ::std::string &regex) { Init(regex.c_str()); } // NOLINT

  RE(const char *regex) { Init(regex); } // NOLINT
  ~RE();

  // Returns the string representation of the regex.
  const char *pattern() const { return pattern_.c_str(); }

  // FullMatch(str, re) returns true if and only if regular expression re-
  // matches the entire str.
  // PartialMatch(str, re) returns true if and only if regular expression re-
  // matches a substring of str (including str itself).
  static bool FullMatch(const ::std::string &str, const RE &re) {
    return FullMatch(str.c_str(), re);
  }
  static bool PartialMatch(const ::std::string &str, const RE &re) {
    return PartialMatch(str.c_str(), re);
  }

  static bool FullMatch(const char *str, const RE &re);
  static bool PartialMatch(const char *str, const RE &re);

private:
  void Init(const char *regex);
  std::string pattern_;
  bool is_valid_;

#ifdef GTEST_USES_POSIX_RE

  regex_t full_regex_;     // For FullMatch().
  regex_t partial_regex_;  // For PartialMatch().
#endif
};

// Formats a source file path and a line number as they would appear
// in an error message from the compiler used to compile this code.
CTEST_API_ ::std::string FormatFileLocation(const char* file, int line);

// Formats a file location for compiler-independent XML output.
// Although this function is not platform dependent, we put it next to
// FormatFileLocation in order to contrast the two functions.
CTEST_API_ ::std::string FormatCompilerIndependentFileLocation(const char* file,
                                                               int line);
// Defines logging utilities:
//   GTEST_LOG_(severity) - logs messages at the specified severity level. The
//                          message itself is streamed into the macro.
//   LogToStderr()  - directs all log messages to stderr.
//   FlushInfoLog() - flushes informational log messages.
enum GTestLogSeverity { GTEST_INFO, GTEST_WARNING, GTEST_ERROR, GTEST_FATAL };

// Formats log entry severity, provides a stream object for streaming the
// log message, and terminates the message with a newline when going out of
// scope.
class CTEST_API_ CTestLog {
public:
  CTestLog(GTestLogSeverity severity, const char* file, int line);

  // Flushes the buffers and, if severity is GTEST_FATAL, aborts the program.
  ~CTestLog();

  ::std::ostream& CetStream() { return ::std::cerr; }

private:
  const GTestLogSeverity severity_;

  CTestLog(const CTestLog &) = delete;
  CTestLog & operator=(const CTestLog &) = delete;
};


#if !defined(CTEST_LOG_)

#define CTEST_LOG_(severity)                                           \
  ::CTestLog(::CTEST_##severity, \
                                __FILE__, __LINE__)                    \
      .GetStream()

inline void LogToStderr() {}
inline void FlushInfoLog() { fflush(nullptr); }

#endif  // !defined(GTEST_LOG_)

#endif // CMOCKERY_CTEST_PORT_H
