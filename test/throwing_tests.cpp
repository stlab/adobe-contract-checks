#include "adobe/contract_checks.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <string_view>

[[noreturn]] void ::adobe::contract_violated(const char *const condition,
  ::adobe::contract_violation::kind_t kind,
  const char *file,
  std::uint32_t line,
  const char *message)
{
  throw contract_violation(condition, kind, file, line, message);
}

TEST(Throwing, PreconditionEncodesExpectedInfo)
{
  std::uint32_t expected_line = 0;
  try {
    expected_line = __LINE__ + 1;
    ADOBE_PRECONDITION(false);
  } catch (const adobe::contract_violation &v) {
    EXPECT_EQ(v.line(), expected_line);
    EXPECT_EQ(std::string_view(v.file()), __FILE__);
    EXPECT_EQ(std::string_view(v.condition()), "false");
  }
}

TEST(Throwing, ThrowingViolationHandlerWorks)
{
  EXPECT_THROW([] { ADOBE_PRECONDITION(false); }(), adobe::contract_violation);

  try {
    ADOBE_PRECONDITION(false, "expected message");
  } catch (adobe::contract_violation const &e) {
    EXPECT_EQ(e.what(), std::string_view("expected message"));
  }
}
