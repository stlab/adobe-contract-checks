#include "adobe/contract_checks.hpp"
#ifndef _WIN32
#include <csignal>
#endif
#include <gtest/gtest.h>

ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()

TEST(PreconditionDeathTest, PreconditionFailureAborts)
{
  EXPECT_EXIT(ADOBE_PRECONDITION(false), testing::KilledBySignal(SIGABRT), "precondition");
}

#ifdef _WIN32
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_ABORT(code, expected_output_regex) \
  EXPECT_DEATH(code, expected_output_regex ".*##ABORTED##");
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXPECT_ABORT(code, expected_output_regex) \
  EXPECT_EXIT(code, testing::KilledBySignal(SIGABRT), expected_output_regex ".*##ABORTED##");
#endif

TEST(PreconditionDeathTest, PreconditionFailureOutput)
{
#line 9998
  EXPECT_ABORT(
    ADOBE_PRECONDITION(false), "precondition_tests.cpp:9999: Precondition violated [(]false[)]");
}

TEST(PreconditionDeathTest, PreconditionFailureWithMessageOutput)
{
#line 99991
  EXPECT_ABORT(ADOBE_PRECONDITION(false, "expected message"),
    "precondition_tests.cpp:99991: Precondition violated [(]false[)]. "
    "expected message");
}
