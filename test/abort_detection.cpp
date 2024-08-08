// When compiled into an executable, this file causes abort() to print
// a special message to stderr and exit with EXIT_FAILURE instead of
// having its usual behavior, allowing our tests to detect that
// abort() was called by checking for the message.
#include <csignal>
#include <cstdio>
#include <cstdlib>

// A signal handler that prints "##ABORTED##" to stderr and exits with
// EXIT_FAILURE.
//
// The printed string is chosen to be unlikely to appear by accident
// in other output.
extern "C" void error_test_handle_abort(int /* unused signum */)
{
  (void)std::fprintf(stderr, "##ABORTED##");// NOLINT
  std::_Exit(EXIT_FAILURE);
}

// Abort handler installer.
struct test_override_abort
{

  // As a side-effect, installs error_test_handle_abort as a SIGABRT
  // handler.
  test_override_abort() noexcept { (void)std::signal(SIGABRT, error_test_handle_abort); }
};

// The installation of error_test_handle_abort as an abort handler.
const test_override_abort handler{};
