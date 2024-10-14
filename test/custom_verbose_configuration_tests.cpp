#include "adobe/contract_checks.hpp"
#include "portable_death_tests.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexcept>

[[noreturn]] void adobe_contract_violated_verbose(const char *condition,
  adobe::contract_violation_kind kind,
  const char *file,
  std::uint32_t line,
  const char *message)
{
  std::array<char, 1024> output{};
  (void)std::snprintf(output.data(),
    sizeof(output),
    "%s:%d: %s (%s). %s\n",
    file,
    static_cast<int>(line),
    kind == adobe::contract_violation_kind::precondition ? "Precondition violated"
                                                         : "Invariant not upheld",
    condition,
    message);
  throw std::logic_error(output.data());
}

namespace {

template<class F> void expect_throw(F f, const char *match)
{
  try {
    f();
  } catch (const std::exception &e) {
    EXPECT_THAT(e.what(), testing::ContainsRegex(match));
    return;
  }
}

}// namespace

#define STRINGIZE(x) STRINGIZE2(x)// NOLINT(cppcoreguidelines-macro-usage)
#define STRINGIZE2(x) #x// NOLINT(cppcoreguidelines-macro-usage)
#define LINE_STRING STRINGIZE(__LINE__)// NOLINT(cppcoreguidelines-macro-usage)

TEST(CustomVerboseConfigurationDeathTest, OneArgumentFormsAbortWithCorrectOutput)
{
  // clang-format off
  expect_throw([] { ADOBE_PRECONDITION(false); }, "custom_verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Precondition violated \\(false\\)\\. \n");
  expect_throw([] { ADOBE_INVARIANT(false); }, "custom_verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Invariant not upheld \\(false\\)\\. \n");
  // clang-format on
}

TEST(CustomVerboseConfigurationDeathTest, TwoArgumentFormsAbortWithCorrectOutput)
{
  // clang-format off
  expect_throw([] { ADOBE_PRECONDITION(false, "% Message %"); }, "custom_verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Precondition violated \\(false\\)\\. % Message %\n"); 
  expect_throw([] { ADOBE_INVARIANT(false, "% Message %"); }, "custom_verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Invariant not upheld \\(false\\)\\. % Message %\n");
  // clang-format on
}

TEST(CustomVerboseConfiguration, ContractNonViolationsDoNotCauseAbort)
{
  ADOBE_PRECONDITION(true);
  ADOBE_INVARIANT(true);
  ADOBE_PRECONDITION(true, "% Message %");
  ADOBE_INVARIANT(true, "% Message %");
}
