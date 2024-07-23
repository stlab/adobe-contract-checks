// When compiled into an executable, this file causes abort() to print
// a special message to stderr and exit with EXIT_FAILURE instead of
// having its usual behavior, allowing our tests to detect that
// abort() was called by checking for the message.
#include <cstdlib>
#include <csignal>
#include <cstdio>

// A signal handler that prints "##ABORTED##" to stderr and exits with
// EXIT_FAILURE.
extern "C" void error_test_handle_abort(int) {
  std::fprintf(stderr, "##ABORTED##");
  std::_Exit(EXIT_FAILURE);
}

// Abort handler installer.
struct test_override_abort {

  // As a side-effect, installs error_test_handle_abort as a SIGABRT
  // handler.
  test_override_abort() noexcept {
    std::signal(SIGABRT, error_test_handle_abort);
  }
};

// The installation of error_test_handle_abort as an abort handler.
const test_override_abort handler{};
