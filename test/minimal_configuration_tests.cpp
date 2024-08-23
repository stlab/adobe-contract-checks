#include "adobe/contract_checks.hpp"
#include "portable_death_tests.hpp"
#include <gtest/gtest.h>

// ****** Death tests should precede non-death tests. *********
//
// For unknown reasons if the last test is a death test, under
// emscripten, the test sometimes fails even if the executable aborts.
// https://gitlab.kitware.com/cmake/cmake/-/issues/26218

TEST(MinimalConfigurationDeathTests, FailedChecksDie)
{
  EXPECT_PORTABLE_DEATH(ADOBE_PRECONDITION(false), "");
  EXPECT_PORTABLE_DEATH(ADOBE_INVARIANT(false), "");

  EXPECT_PORTABLE_DEATH(ADOBE_PRECONDITION(false, "#~#"), "");
  EXPECT_PORTABLE_DEATH(ADOBE_INVARIANT(false, "#~#"), "");
}

TEST(MinimalConfiguration, ContractNonViolationsDoNotCauseAbort)
{
  ADOBE_PRECONDITION(true);
  ADOBE_INVARIANT(true);

  ADOBE_PRECONDITION(true, "##########");
  ADOBE_INVARIANT(true, "#########");
}

#if defined(__EMSCRIPTEN__) && 0
TEST(MinimalConfiguration, EmscriptenDummy) {}
#endif
