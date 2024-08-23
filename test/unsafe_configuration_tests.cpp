#include "adobe/contract_checks.hpp"
#include "portable_death_tests.hpp"
#include <gtest/gtest.h>

TEST(UnsafeConfiguration, AllChecksAreNoOps)
{
  ADOBE_PRECONDITION(true);
  ADOBE_INVARIANT(true);
  ADOBE_PRECONDITION(false);
  ADOBE_INVARIANT(false);

  ADOBE_PRECONDITION(true, "whatever");
  ADOBE_INVARIANT(true, "ignored");
  ADOBE_PRECONDITION(false, "string");
  ADOBE_INVARIANT(false, "here");
}

// ****** This should be the last test in the file. *********
//
// For unknown reasons if the last test is a death test, under
// emscripten, the test fails even if the executable aborts.
#if defined(__EMSCRIPTEN__)
TEST(MinimalConfiguration, EmscriptenDummy) {}
#endif
