#include <cstdlib>
#include <csignal>
#include <cstdio>

// This is a hack to implement death tests in CTest.
extern "C" void error_test_handle_abort(int) {
  std::fprintf(stderr, "##ABORTED##");
  // Because the tests use PASS_REGULAR_EXPRESSION, this exit code
  // will be ignored.
  std::_Exit(EXIT_FAILURE);
}

struct test_override_abort {
  test_override_abort() noexcept {
    std::signal(SIGABRT, error_test_handle_abort);
  }
};

test_override_abort handler{};
