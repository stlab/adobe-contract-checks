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

TEST(DefaultHandler, ContractNonViolationsDoNotCauseAbort)
{
  ADOBE_PRECONDITION(true);
  ADOBE_POSTCONDITION(true);
  ADOBE_INVARIANT(true);
}

TEST(DefaultHandlerDeathTest, ContractViolationsCauseAbort)
{
  EXPECT_ABORT(ADOBE_PRECONDITION(false), "");
  EXPECT_ABORT(ADOBE_POSTCONDITION(false), "");
  EXPECT_ABORT(ADOBE_INVARIANT(false), "");
}

#define STRINGIZE(x) STRINGIZE2(x)// NOLINT(cppcoreguidelines-macro-usage)
#define STRINGIZE2(x) #x// NOLINT(cppcoreguidelines-macro-usage)
#define LINE_STRING STRINGIZE(__LINE__)// NOLINT(cppcoreguidelines-macro-usage)

TEST(DefaultHandlerDeathTest, OneArgumentFormsAbortWithCorrectOutput)
{
  // clang-format off
  EXPECT_ABORT(ADOBE_PRECONDITION(false), "default_handler_tests\\.cpp:" LINE_STRING
               ": Precondition violated \\(false\\)\\. \n");
  EXPECT_ABORT(ADOBE_POSTCONDITION(false), "default_handler_tests\\.cpp:" LINE_STRING
               ": Postcondition not upheld \\(false\\)\\. \n");
  EXPECT_ABORT(ADOBE_INVARIANT(false), "default_handler_tests\\.cpp:" LINE_STRING
               ": Invariant not upheld \\(false\\)\\. \n");
  // clang-format on
}

TEST(DefaultHandlerDeathTest, TwoArgumentFormsAbortWithCorrectOutput)
{
  // clang-format off
  EXPECT_ABORT(ADOBE_PRECONDITION(false, "% Message %"), "default_handler_tests\\.cpp:" LINE_STRING
               ": Precondition violated \\(false\\)\\. % Message %\n");
  EXPECT_ABORT(ADOBE_POSTCONDITION(false, "% Message %"), "default_handler_tests\\.cpp:" LINE_STRING
                 ": Postcondition not upheld \\(false\\)\\. % Message %\n");
  EXPECT_ABORT(ADOBE_INVARIANT(false, "% Message %"), "default_handler_tests\\.cpp:" LINE_STRING
               ": Invariant not upheld \\(false\\)\\. % Message %\n");
  // clang-format on
}
