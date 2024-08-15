#include "adobe/contract_checks.hpp"
#include <gtest/gtest.h>
#if !defined(GTEST_OS_WINDOWS) && !defined(__EMSCRIPTEN__)
#include <csignal>
#endif

ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()

#if defined(__EMSCRIPTEN__)

// GoogleTest doesn't support death tests under emscripten, so instead
// we handle death by setting the test's WILL_FAIL property in CMake.
// Therefore the test simply executes the code that aborts with no
// wrapper.  There's currently no facility for checking test output.

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_ABORT(code, expected_output_regex) code

#elif defined(GTEST_OS_WINDOWS)
// GoogleTest doesn't support checking for the abort signal on
// Windows, so we use an auxilliary file, win32_abort_detection.cpp,
// to ensure that an unusual string is printed, which we can check for
// with the EXPECT_DEATH macro.

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_ABORT(code, expected_output_regex) \
  EXPECT_DEATH(code, expected_output_regex ".*\n*##ABORTED##");

#else

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_ABORT(code, expected_output_regex) \
  EXPECT_EXIT(code, testing::KilledBySignal(SIGABRT), expected_output_regex);

#endif

TEST(PreconditionDeathTest, PreconditionFailureAborts)
{
  EXPECT_ABORT(ADOBE_PRECONDITION(false), "Precondition violated \\(false\\)\\.");
}

TEST(PreconditionDeathTest, PreconditionFailureOutput)
{
#line 998
  EXPECT_ABORT(
    ADOBE_PRECONDITION(false), "precondition_tests.cpp:999: Precondition violated \\(false\\)\\.");
}

TEST(PreconditionDeathTest, PreconditionFailureWithMessageOutput)
{
#line 9991
  EXPECT_ABORT(ADOBE_PRECONDITION(false, "expected message"),
    "precondition_tests.cpp:999.: Precondition violated \\(false\\)\\. expected message");
}

// ****** This should be the last test in the file. *********
//
// For unknown reasons if the last test is a death test, under
// emscripten, the test fails even if the executable aborts.
#if defined(__EMSCRIPTEN__)
TEST(Precondition, EmscriptenDummy) {}
#endif
