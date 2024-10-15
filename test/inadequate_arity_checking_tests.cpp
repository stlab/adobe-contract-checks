#include "adobe/contract_checks.hpp"
#include <gtest/gtest.h>

namespace {

struct Failure
{
  template<class T, class U, class V, class W>
  void operator()(const T & /*unused*/,
    const U & /*unused*/,
    const V & /*unused*/,
    const W & /*unused*/) const
  {}
};

const Failure failure;

}// namespace

TEST(ArityChecking, TooManyArgumentsNotAlwaysDetected)
{
  // These don't fail, so our arity detection is weak.
  // https://github.com/stlab/adobe-contract-checks/issues/19
  ADOBE_PRECONDITION(true, "message", failure);
  ADOBE_INVARIANT(true, "message", failure);
}
