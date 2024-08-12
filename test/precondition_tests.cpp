#include "adobe/contract_checks.hpp"
#include <csignal>
#include <gtest/gtest.h>

ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()

TEST(PreconditionDeathTest, PreconditionFailureAborts)
{
  EXPECT_EXIT(ADOBE_PRECONDITION(false), testing::KilledBySignal(SIGABRT), "precondition");
}

TEST(PreconditionDeathTest, PreconditionFailureOutput)
{
#line 9999
  EXPECT_EXIT(ADOBE_PRECONDITION(false),
    testing::KilledBySignal(SIGABRT),
    "precondition_tests.cpp:9999: Precondition violated [(]false[)]");
}

TEST(PreconditionDeathTest, PreconditionFailureWithMessageOutput)
{
#line 99991
  EXPECT_EXIT(ADOBE_PRECONDITION(false, "expected message"),
    testing::KilledBySignal(SIGABRT),
    "precondition_tests.cpp:99991: Precondition violated [(]false[)]. "
    "expected message");
}
