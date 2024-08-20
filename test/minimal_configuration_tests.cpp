#include "adobe/contract_checks.hpp"
#include "portable_death_tests.hpp"
#include <gtest/gtest.h>

TEST(MinimalConfigurationDeathTests, FailedChecksDie)
{
  EXPECT_PORTABLE_DEATH(ADOBE_PRECONDITION(false), "");
  EXPECT_PORTABLE_DEATH(ADOBE_POSTCONDITION(false), "");
  EXPECT_PORTABLE_DEATH(ADOBE_INVARIANT(false), "");
}

// ****** This should be the last test in the file. *********
//
// For unknown reasons if the last test is a death test, under
// emscripten, the test fails even if the executable aborts.
#if defined(__EMSCRIPTEN__)
TEST(MinimalConfiguration, EmscriptenDummy) {}
#endif
