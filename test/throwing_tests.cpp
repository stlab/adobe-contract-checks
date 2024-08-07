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

int main()
{
  std::uint32_t expected_line = 0;
  try {
    expected_line = __LINE__ + 1;
    ADOBE_PRECONDITION(false);
  } catch (const adobe::contract_violation &v) {
    assert((v.line() == expected_line));
    assert((std::string_view(v.file()) == __FILE__));
    assert((std::string_view(v.condition()) == "false"));
  }

  try {
    expected_line = __LINE__ + 1;
    ADOBE_PRECONDITION(false, "message");
  } catch (const adobe::contract_violation &v) {
    assert((v.line() == expected_line));
    assert((std::string_view(v.file()) == __FILE__));
    assert((std::string_view(v.condition()) == "false"));
    assert((std::string_view(v.what()) == "message"));
  }

  fprintf(stderr, "passed\n");
}
