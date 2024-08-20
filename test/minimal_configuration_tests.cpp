#include "adobe/contract_checks.hpp"
#include "portable_death_tests.hpp"
#include <gtest/gtest.h>

TEST(MinimalConfigurationDeathTests, FailedChecksDie)
{
  EXPECT_PORTABLE_DEATH(ADOBE_PRECONDITION(false), "");
  EXPECT_PORTABLE_DEATH(ADOBE_POSTCONDITION(false), "");
  EXPECT_PORTABLE_DEATH(ADOBE_INVARIANT(false), "");
}
