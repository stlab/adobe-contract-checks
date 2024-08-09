#include "adobe/contract_checks.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <cstdint>
#include <string_view>

[[noreturn]] void ::adobe::contract_violated(const char *const condition,
  ::adobe::contract_violation::kind_t kind,
  const char *file,
  std::uint32_t line,
  const char *message)
{
  throw contract_violation(condition, kind, file, line, message);
}

TEST_CASE("Precondition encodes expected info", "[expected-info-encoded]")
{
  std::uint32_t expected_line = 0;
  try {
    expected_line = __LINE__ + 1;
    ADOBE_PRECONDITION(false);
  } catch (const adobe::contract_violation &v) {
    CHECK((v.line() == expected_line));
    CHECK((std::string_view(v.file()) == __FILE__));
    CHECK((std::string_view(v.condition()) == "false"));
  }
}

#include <cstdio>

TEST_CASE("Throwing violation handler works", "[throwing]")
{
  std::printf("step zero");// NOLINT
  std::fflush(stdout);

  CHECK_THROWS_AS([] { ADOBE_PRECONDITION(false); }(), adobe::contract_violation);
  std::printf("step one");// NOLINT
  std::fflush(stdout);

  CHECK_THROWS_WITH([] { ADOBE_PRECONDITION(false, "expected message"); }(), "expected message");
  std::printf("step two");// NOLINT
  std::fflush(stdout);
}
