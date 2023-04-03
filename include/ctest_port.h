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
#define GTEST_AMBIGUOUS_ELSE_BLOCKER_ \
  switch (0)                          \
  case 0:                             \
  default:  // NOLINT



// Determines the version of gcc that is used to compile this.
#ifdef __GNUC__
// 40302 means version 4.3.2.
#define GTEST_GCC_VER_ \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif  // __GNUC__

//正则表达式匹配
#include <condition_variable>
#include <mutex>
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
enum GTestLogSeverity { CTEST_INFO, CTEST_WARNING, CTEST_ERROR, CTEST_FATAL };

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
      .CetStream()

inline void LogToStderr() {}
inline void FlushInfoLog() { fflush(nullptr); }

#endif  // !defined(GTEST_LOG_)


#if !defined(GTEST_CHECK_)
// INTERNAL IMPLEMENTATION - DO NOT USE.
//
// GTEST_CHECK_ is an all-mode assert. It aborts the program if the condition
// is not satisfied.
//  Synopsis:
//    GTEST_CHECK_(boolean_condition);
//     or
//    GTEST_CHECK_(boolean_condition) << "Additional message";
//
//    This checks the condition and if the condition is not satisfied
//    it prints message about the condition violation, including the
//    condition itself, plus additional message streamed into it, if any,
//    and then it aborts the program. It aborts the program irrespective of
//    whether it is built in the debug mode or not.
#define GTEST_CHECK_(condition)               \
  GTEST_AMBIGUOUS_ELSE_BLOCKER_               \
  if (IsTrue(condition)) \
    ;                                         \
  else                                        \
    CTEST_LOG_(FATAL) << "Condition " #condition " failed. "
#endif  // !defined(GTEST_CHECK_)

// An all-mode assert to verify that the given POSIX-style function
// call returns 0 (indicating success).  Known limitation: this
// doesn't expand to a balanced 'if' statement, so enclose the macro
// in {} if you need to use it as the only statement in an 'if'
// branch.用于验证给定的 POSIX 样式函数调用是否返回 0（指示成功）的全模式断言。已知限制：
// 这不会扩展到平衡的“if”语句，因此如果您需要将其用作“if”分支中的唯一语句，请将宏括在 {} 中。
#define GTEST_CHECK_POSIX_SUCCESS_(posix_call) \
  { if (const int gtest_error = (posix_call))    \
  CTEST_LOG_(FATAL) << #posix_call << "failed with error " << gtest_error; }


#if GTEST_HAS_STREAM_REDIRECTION
// Defines the stderr capturer:
//   CaptureStdout     - starts capturing stdout.
//   GetCapturedStdout - stops capturing stdout and returns the captured string.
//   CaptureStderr     - starts capturing stderr.
//   GetCapturedStderr - stops capturing stderr and returns the captured string.
//
CTEST_API_ void CaptureStdout();
CTEST_API_ std::string GetCapturedStdout();
CTEST_API_ void CaptureStderr();
CTEST_API_ std::string GetCapturedStderr();

#endif  // GTEST_HAS_STREAM_REDIRECTION


// Allows a controller thread to pause execution of newly created
// threads until notified.  Instances of this class must be created
// and destroyed in the controller thread.
//
// This class is only for testing Google Test's own constructs. Do not
// use it in user tests, either directly or indirectly.
// TODO(b/203539622): Replace unconditionally with absl::Notification.
class CTEST_API_ Notification {
public:
  Notification() : notified_(false) {}
  Notification(const Notification&) = delete;
  Notification& operator=(const Notification&) = delete;

  // Notifies all threads created with this notification to start. Must
  // be called from the controller thread.
  void Notify() {
    std::lock_guard<std::mutex> lock(mu_);
    notified_ = true;
    cv_.notify_all();
  }

  // Blocks until the controller thread notifies. Must be called from a test
  // thread.
  void WaitForNotification() {
    std::unique_lock<std::mutex> lock(mu_);
    cv_.wait(lock, [this]() { return notified_; });
  }

private:
  std::mutex mu_;
  std::condition_variable cv_;
  bool notified_;
};

// As a C-function, ThreadFuncWithCLinkage cannot be templated itself.
// Consequently, it cannot select a correct instantiation of ThreadWithParam
// in order to call its Run(). Introducing ThreadWithParamBase as a
// non-templated base class for ThreadWithParam allows us to bypass this
// problem.
class ThreadWithParamBase {
public:
  virtual ~ThreadWithParamBase() {}
  virtual void Run() = 0;
};

// pthread_create() accepts a pointer to a function type with the C linkage.
// According to the Standard (7.5/1), function types with different linkages
// are different even if they are otherwise identical.  Some compilers (for
// example, SunStudio) treat them as different types.  Since class methods
// cannot be defined with C-linkage we need to define a free C-function to
// pass into pthread_create().
extern "C" inline void* ThreadFuncWithCLinkage(void* thread) {
  static_cast<ThreadWithParamBase*>(thread)->Run();
  return nullptr;
}


// Helper class for testing Google Test's multi-threading constructs.
// To use it, write:
//
//   void ThreadFunc(int param) { /* Do things with param */ }
//   Notification thread_can_start;
//   ...
//   // The thread_can_start parameter is optional; you can supply NULL.
//   ThreadWithParam<int> thread(&ThreadFunc, 5, &thread_can_start);
//   thread_can_start.Notify();
//
// These classes are only for testing Google Test's own constructs. Do
// not use them in user tests, either directly or indirectly.
template <typename T>
class ThreadWithParam : public ThreadWithParamBase {
public:
  typedef void UserThreadFunc(T);

  ThreadWithParam(UserThreadFunc* func, T param, Notification* thread_can_start)
      : func_(func),
        param_(param),
        thread_can_start_(thread_can_start),
        finished_(false) {
    ThreadWithParamBase* const base = this;
    // The thread can be created only after all fields except thread_
    // have been initialized.
    GTEST_CHECK_POSIX_SUCCESS_(
        pthread_create(&thread_, nullptr, &ThreadFuncWithCLinkage, base));
  }
  ~ThreadWithParam() override { Join(); }

  void Join() {
    if (!finished_) {
      GTEST_CHECK_POSIX_SUCCESS_(pthread_join(thread_, nullptr));
      finished_ = true;
    }
  }

  void Run() override {
    if (thread_can_start_ != nullptr) thread_can_start_->WaitForNotification();
    func_(param_);
  }

private:
  UserThreadFunc* const func_;  // User-supplied thread function.
  const T param_;  // User-supplied parameter to the thread function.
  // When non-NULL, used to block execution until the controller thread
  // notifies.
  Notification* const thread_can_start_;
  bool finished_;  // true if and only if we know that the thread function has
                     // finished.
  pthread_t thread_;  // The native thread object.

  ThreadWithParam(const ThreadWithParam&) = delete;
  ThreadWithParam& operator=(const ThreadWithParam&) = delete;
};

#endif // CMOCKERY_CTEST_PORT_H
