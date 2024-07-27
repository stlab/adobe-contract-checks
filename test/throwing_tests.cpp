#include "adobe/contract_checks.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <cstdint>
#include <string_view>
#include <system_error>

[[noreturn]] void ::adobe::contract_violated(::adobe::contract_violation_kind condition,
  const char *file,
  std::uint32_t line,
  const char *message)
{
  throw ::adobe::contract_violation(condition, file, line, message);
}

TEST_CASE("Precondition violation encodes file and line", "[file-and-line-encoded]")
{
  std::uint32_t expected_line = 0;
  try {
    expected_line = __LINE__ + 1;
    ADOBE_PRECONDITION(false);
  } catch (const adobe::contract_violation &v) {
    CHECK((v.line() == expected_line));
    CHECK((std::string_view(v.file()) == __FILE__));
  }
}

TEST_CASE("Throwing violation handler works", "[throwing]")
{
  CHECK_THROWS_AS([] { ADOBE_PRECONDITION(false); }(), adobe::contract_violation);
  CHECK_THROWS_WITH([] { ADOBE_PRECONDITION(false); }(), "precondition");
}
