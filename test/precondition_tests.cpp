#include "adobe/contract_checks.hpp"
#include <gtest/gtest.h>
#if !defined(GTEST_OS_WINDOWS) && !defined(GTEST_OS_FUCHSIA)
#include <csignal>
#endif

ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()

#if defined(GTEST_OS_WINDOWS) || defined(GTEST_OS_FUCHSIA)
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
