#include "adobe/contract_checks.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

[[noreturn]] void adobe_contract_violated_lightweight()
{
  throw std::logic_error("adobe_contract_violated_lightweight");
}

TEST(CustomLightweightConfiguration, FailedChecksThrow)
{
  EXPECT_THROW(ADOBE_PRECONDITION(false), std::logic_error);
  EXPECT_THROW(ADOBE_INVARIANT(false), std::logic_error);

  EXPECT_THROW(ADOBE_PRECONDITION(false, "##########"), std::logic_error);
  EXPECT_THROW(ADOBE_INVARIANT(false, "#########"), std::logic_error);
}

TEST(CustomLightweightConfiguration, ContractNonViolationsDoNotThrow)
{
  ADOBE_PRECONDITION(true);
  ADOBE_INVARIANT(true);

  ADOBE_PRECONDITION(true, "##########");
  ADOBE_INVARIANT(true, "#########");
}
