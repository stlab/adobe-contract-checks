#include "adobe/contract_checks.hpp"
#include <gtest/gtest.h>

struct F
{
  template<class T, class U, class V, class W>
  void operator()(const T & /*unused*/,
    const U & /*unused*/,
    const V & /*unused*/,
    const W & /*unused*/) const
  {}
};
const F f;

TEST(ArityChecking, TooManyArgumentsNotAlwaysDetected)
{
  // These don't fail, so our arity detection is weak.
  // https://github.com/stlab/adobe-contract-checks/issues/19
  ADOBE_PRECONDITION(true, "message", f);
  ADOBE_INVARIANT(true, "message", f);
}
