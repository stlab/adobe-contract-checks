#include "adobe/contract_checks.hpp"
#include <cstdint>
#include <string_view>

#undef NDEBUG
#include <cassert>

[[noreturn]] void ::adobe::contract_violated(const char *const condition,
  ::adobe::contract_violation::kind_t kind,
  const char *file,
  std::uint32_t line,
  const char *message)
{
  throw contract_violation(condition, kind, file, line, message);
}

#define ASSERT(condition)                                             \
  if (condition) {                                                    \
  } else {                                                            \
    (void)std::fprintf(stderr, "Assertion failed: %s\n", #condition); \
    (void)std::fflush(stderr);                                        \
    std::abort();                                                     \
  }

int main()
{
  std::uint32_t expected_line = 0;
  try {
    expected_line = __LINE__ + 1;
    ADOBE_PRECONDITION(false);
  } catch (const adobe::contract_violation &v) {
    ASSERT((v.line() == expected_line));
    ASSERT((std::string_view(v.file()) == __FILE__));
    ASSERT((std::string_view(v.condition()) == "false"));
  }

  try {
    expected_line = __LINE__ + 1;
    ADOBE_PRECONDITION(false, "message");
  } catch (const adobe::contract_violation &v) {
    ASSERT((v.line() == expected_line));
    ASSERT((std::string_view(v.file()) == __FILE__));
    ASSERT((std::string_view(v.condition()) == "false"));
    ASSERT((std::string_view(v.what()) == "message"));
  }

  (void)fprintf(stderr, "passed\n");
  (void)fflush(stderr);
}
