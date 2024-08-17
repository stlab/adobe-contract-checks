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

TEST(Throwing, ThrowingViolationHandlerWorks)
{
  EXPECT_THROW([] { ADOBE_PRECONDITION(false); }(), adobe::contract_violation);
  EXPECT_THROW([] { ADOBE_POSTCONDITION(false); }(), adobe::contract_violation);
  EXPECT_THROW([] { ADOBE_INVARIANT(false); }(), adobe::contract_violation);
}

TEST(Throwing, OneArgumentFormsEncodeExpectedInfo)
{
  std::uint32_t expected_line = 0;
  try {
    expected_line = __LINE__ + 1;
    ADOBE_PRECONDITION(false);
  } catch (const adobe::contract_violation &v) {
    EXPECT_EQ(v.kind(), ::adobe::contract_violation::predefined_kind::precondition);
    EXPECT_EQ(v.line(), expected_line);
    EXPECT_EQ(std::string_view(v.file()), __FILE__);
    EXPECT_EQ(std::string_view(v.condition()), "false");
    EXPECT_EQ(std::string_view(v.what()), "");
  }

  try {
    expected_line = __LINE__ + 1;
    ADOBE_POSTCONDITION(false);
  } catch (const adobe::contract_violation &v) {
    EXPECT_EQ(v.kind(), ::adobe::contract_violation::predefined_kind::postcondition);
    EXPECT_EQ(v.line(), expected_line);
    EXPECT_EQ(std::string_view(v.file()), __FILE__);
    EXPECT_EQ(std::string_view(v.condition()), "false");
    EXPECT_EQ(std::string_view(v.what()), "");
  }

  try {
    expected_line = __LINE__ + 1;
    ADOBE_INVARIANT(false);
  } catch (const adobe::contract_violation &v) {
    EXPECT_EQ(v.kind(), ::adobe::contract_violation::predefined_kind::invariant);
    EXPECT_EQ(v.line(), expected_line);
    EXPECT_EQ(std::string_view(v.file()), __FILE__);
    EXPECT_EQ(std::string_view(v.condition()), "false");
    EXPECT_EQ(std::string_view(v.what()), "");
  }
}

TEST(Throwing, TwoArgumentFormsEncodeExpectedInfo)
{
  std::uint32_t expected_line = 0;
  try {
    expected_line = __LINE__ + 1;
    ADOBE_PRECONDITION(false, "% message %");
  } catch (const adobe::contract_violation &v) {
    EXPECT_EQ(v.kind(), ::adobe::contract_violation::predefined_kind::precondition);
    EXPECT_EQ(v.line(), expected_line);
    EXPECT_EQ(std::string_view(v.file()), __FILE__);
    EXPECT_EQ(std::string_view(v.condition()), "false");
    EXPECT_EQ(std::string_view(v.what()), "% message %");
  }

  try {
    expected_line = __LINE__ + 1;
    ADOBE_POSTCONDITION(false, "% message %");
  } catch (const adobe::contract_violation &v) {
    EXPECT_EQ(v.kind(), ::adobe::contract_violation::predefined_kind::postcondition);
    EXPECT_EQ(v.line(), expected_line);
    EXPECT_EQ(std::string_view(v.file()), __FILE__);
    EXPECT_EQ(std::string_view(v.condition()), "false");
    EXPECT_EQ(std::string_view(v.what()), "% message %");
  }

  try {
    expected_line = __LINE__ + 1;
    ADOBE_INVARIANT(false, "% message %");
  } catch (const adobe::contract_violation &v) {
    EXPECT_EQ(v.kind(), ::adobe::contract_violation::predefined_kind::invariant);
    EXPECT_EQ(v.line(), expected_line);
    EXPECT_EQ(std::string_view(v.file()), __FILE__);
    EXPECT_EQ(std::string_view(v.condition()), "false");
    EXPECT_EQ(std::string_view(v.what()), "% message %");
  }
}

