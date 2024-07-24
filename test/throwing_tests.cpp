#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include "adobe/contract_checks.hpp"

[[noreturn]] void ::adobe::contract_violated(::adobe::contract_violation const& reason) {
  throw reason;
}

TEST_CASE("Throwing violation handler works", "[throwing]")
{
  CHECK_THROWS_AS([]{ ADOBE_PRECONDITION(false); }(), adobe::contract_violation);
  CHECK_THROWS_WITH([]{ ADOBE_PRECONDITION(false); }(), "precondition");
}
