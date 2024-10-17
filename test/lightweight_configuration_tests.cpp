#include "adobe/contract_checks.hpp"
#include "portable_death_tests.hpp"
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

// ****** Death tests should precede non-death tests. *********
//
// For unknown reasons if the last test is a death test, under
// emscripten, the test sometimes fails even if the executable aborts.
// https://gitlab.kitware.com/cmake/cmake/-/issues/26218

using ::testing::HasSubstr;
using ::testing::Not;

TEST(LightweightConfigurationDeathTest, FailedChecksDie)
{
  const bool uNlIKeLyIdEnTiFiEr = false;

  EXPECT_PORTABLE_DEATH(
    ADOBE_PRECONDITION(uNlIKeLyIdEnTiFiEr), Not(HasSubstr("uNlIKeLyIdEnTiFiEr")));
  EXPECT_PORTABLE_DEATH(ADOBE_INVARIANT(uNlIKeLyIdEnTiFiEr), Not(HasSubstr("uNlIKeLyIdEnTiFiEr")));

  EXPECT_PORTABLE_DEATH(
    ADOBE_PRECONDITION(false, "~uNlIKeLyIdEnTiFiEr~"), Not(HasSubstr("~uNlIKeLyIdEnTiFiEr~")));
  EXPECT_PORTABLE_DEATH(
    ADOBE_INVARIANT(false, "~uNlIKeLyIdEnTiFiEr~"), Not(HasSubstr("~uNlIKeLyIdEnTiFiEr~")));
}

TEST(LightweightConfiguration, ContractNonViolationsDoNotCauseAbort)
{
  ADOBE_PRECONDITION(true);
  ADOBE_INVARIANT(true);

  ADOBE_PRECONDITION(true, "% Message %"");
  ADOBE_INVARIANT(true, "% Message %");
}

#if defined(__EMSCRIPTEN__) && 0
TEST(LightweightConfiguration, EmscriptenDummy) {}
#endif
