#include "adobe/contract_checks.hpp"
#include "portable_death_tests.hpp"
#include <gtest/gtest.h>

// ****** Death tests should precede non-death tests. *********
//
// For unknown reasons if the last test is a death test, under
// emscripten, the test sometimes fails even if the executable aborts.
// https://gitlab.kitware.com/cmake/cmake/-/issues/26218
TEST(VerboseConfigurationDeathTest, ContractViolationsCauseAbort)
{
  EXPECT_ABORT(ADOBE_PRECONDITION(false), "");
  EXPECT_ABORT(ADOBE_INVARIANT(false), "");
}

#define STRINGIZE(x) STRINGIZE2(x)// NOLINT(cppcoreguidelines-macro-usage)
#define STRINGIZE2(x) #x// NOLINT(cppcoreguidelines-macro-usage)
#define LINE_STRING STRINGIZE(__LINE__)// NOLINT(cppcoreguidelines-macro-usage)

TEST(VerboseConfigurationDeathTest, OneArgumentFormsAbortWithCorrectOutput)
{
  // clang-format off
  EXPECT_ABORT(ADOBE_PRECONDITION(false), "verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Precondition violated \\(false\\)\\. \n");
  EXPECT_ABORT(ADOBE_INVARIANT(false), "verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Invariant not upheld \\(false\\)\\. \n");
  // clang-format on
}

TEST(VerboseConfigurationDeathTest, TwoArgumentFormsAbortWithCorrectOutput)
{
  // clang-format off
  EXPECT_ABORT(ADOBE_PRECONDITION(false, "% Message %"), "verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Precondition violated \\(false\\)\\. % Message %\n");
  EXPECT_ABORT(ADOBE_INVARIANT(false, "% Message %"), "verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Invariant not upheld \\(false\\)\\. % Message %\n");
  // clang-format on
}

TEST(VerboseConfiguration, ContractNonViolationsDoNotCauseAbort)
{
  ADOBE_PRECONDITION(true);
  ADOBE_INVARIANT(true);
}
