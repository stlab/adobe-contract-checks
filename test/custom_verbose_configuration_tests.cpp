#include "adobe/contract_checks.hpp"
#include "portable_death_tests.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexcept>

// Throws a std::logic_error with a message constructed from the arguments.
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

// Expects that `f()` throws an exception and that the exception's `what()` method contains the
// `match` regex.
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

// LINE_STRING turns __LINE__ into a string literal.
#define STRINGIZE(x) STRINGIZE2(x)// NOLINT(cppcoreguidelines-macro-usage)
#define STRINGIZE2(x) #x// NOLINT(cppcoreguidelines-macro-usage)
#define LINE_STRING STRINGIZE(__LINE__)// NOLINT(cppcoreguidelines-macro-usage)

TEST(CustomVerboseConfiguration, OneArgumentFormsCallHandlerWithCorrectArguments)
{
  // clang-format off
  expect_throw([] { ADOBE_PRECONDITION(false); }, "custom_verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Precondition violated \\(false\\)\\. \n");
  expect_throw([] { ADOBE_INVARIANT(false); }, "custom_verbose_configuration_tests\\.cpp:" LINE_STRING
               ": Invariant not upheld \\(false\\)\\. \n");
  // clang-format on
}

TEST(CustomVerboseConfiguration, TwoArgumentFormsCallHandlerWithCorrectArguments)
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