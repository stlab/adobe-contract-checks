#include <gtest/gtest.h>
#if !defined(GTEST_OS_WINDOWS) && !defined(__EMSCRIPTEN__)
#include <csignal>
#endif

// gtest-style test macros that, given the use of handle_emscripten_death_tests in CMakeLists.txt,
// portably detect various kinds of abnormal exits.
//
// EXPECT_ABORT: succeeds when the test case aborts.
// EXPECT_PORTABLE_DEATH: succeeds when the test case exits abnormally.

#if defined(__EMSCRIPTEN__)

// GoogleTest doesn't support death tests under emscripten, so instead
// we handle death by setting the test's WILL_FAIL property in CMake.
// Therefore the test simply executes the code that aborts with no
// wrapper.  There's currently no facility for checking test output.

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_ABORT(code, expected_output_regex) code

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_PORTABLE_DEATH(code, expected_output_regex) code

#elif defined(GTEST_OS_WINDOWS)
// GoogleTest doesn't support checking for the abort signal on
// Windows, so we use an auxilliary file, win32_abort_detection.cpp,
// to ensure that an unusual string is printed, which we can check for
// with the EXPECT_DEATH macro.

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_ABORT(code, expected_output_regex) \
  EXPECT_DEATH(code, expected_output_regex ".*\n*##ABORTED##");

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_PORTABLE_DEATH(code, expected_output_regex) EXPECT_DEATH(code, expected_output_regex)

#else

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_ABORT(code, expected_output_regex) \
  EXPECT_EXIT(code, testing::KilledBySignal(SIGABRT), expected_output_regex);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_PORTABLE_DEATH(code, expected_output_regex) EXPECT_DEATH(code, expected_output_regex)

#endif
