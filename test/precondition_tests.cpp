#include "adobe/contract_checks.hpp"
#ifndef _WIN32
#include <csignal>
#endif
#include <gtest/gtest.h>

ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()

#ifdef _WIN32
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_ABORT(code, expected_output_regex) EXPECT_DEATH(code, expected_output_regex);
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
