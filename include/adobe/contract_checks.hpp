#ifndef ADOBE_CONTRACT_CHECKS_HPP
#define ADOBE_CONTRACT_CHECKS_HPP
#include <cstdint>
#include <cstdio>
#include <exception>
#include <stdexcept>

namespace adobe {

// A violation of some API contract.
class contract_violation final : public ::std::logic_error
{
public:
  using kind_t = int;

  // The predefined kinds of contract violations provided by this library.
  enum predefined_kind : kind_t {
    precondition = 1,
    // A precondition check that dynamically ensures safety failed.
    safety_precondition,
    postcondition,
    invariant,
    unconditional_fatal_error
  };

private:
  // A string representation of the condition whose falsity caused this violation to be detected.
  const char *_condition;

  // The kind of violation.
  kind_t _kind;

  // The file in which the violation occurred.
  const char *_file = "";

  // The line number on which the violation occurred.
  std::uint32_t _line = 0;

public:
  explicit contract_violation(const char *condition,
    kind_t kind,
    const char *file,
    std::uint32_t line,
    const char *message)
    : ::std::logic_error(message), _condition(condition), _kind(kind), _file(file), _line(line)
  {}

  // Returns a string representation of the condition whose falsity caused this violation to be
  // detected.
  const char *condition() const { return _condition; };

  // Returns the file in which the violation occurred.
  const char *file() const { return _file; }

  // Returns the line number on which the violation occurred.
  std::uint32_t line() const { return _line; }

  kind_t kind() const noexcept { return _kind; }

  void print_report() const
  {
    if (_kind == predefined_kind::unconditional_fatal_error) {
      std::fprintf(stderr, "%s:%d: %s: %s\n", _file, _line, "Unconditional fatal error", what());
    } else {
      const char *const description =
        _kind == predefined_kind::precondition || _kind == predefined_kind::safety_precondition
          ? "Precondition violated"
        : _kind == predefined_kind::postcondition ? "Postcondition not upheld"
        : _kind == predefined_kind::invariant     ? "Invariant violated"
                                                  : "Unknown category kind";
      std::fprintf(stderr, "%s:%d: %s (%s). %s\n", _file, _line, description, _condition, what());
    }
  }
};

// The handler for contract violations, defined in the client's code.
//
// A default handler can be injected using ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER().
[[noreturn]] void contract_violated(const char *condition,
  contract_violation::kind_t kind,
  const char *file,
  std::uint32_t line,
  const char *message);

[[noreturn]] inline void default_contract_violated(const char *const condition,
  contract_violation::kind_t kind,
  const char *const file,
  std::uint32_t const line,
  const char *const message)
{
  // This pattern, calling terminate while unwinding, causes most
  // standard libraries to report the exception that was thrown via
  // a default terminate handler.
  try {
    throw contract_violation(condition, kind, file, line, message);
  } catch (contract_violation const &e) {
    e.print_report();
    std::terminate();
  }
}

}// namespace adobe


// TODO: Supply a default terminate handler that calls
// get_current_exception and reports info.

#if __has_cpp_attribute(unlikely)
// The attribute (if any) that marks the cold path in a contract check.
#define ADOBE_CONTRACT_VIOLATION_LIKELIHOOD [[unlikely]]
#else
// The attribute (if any) that marks the cold path in a contract check.
#define ADOBE_CONTRACT_VIOLATION_LIKELIHOOD
#endif

// Injects a definition of ::adobe::contract_violated that reports
// violations to stdout and invokes std::terminate.
//
// See ::adobe::default_contract_violated for details.
#define ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()                            \
  [[noreturn]] void ::adobe::contract_violated(const char *const condition,   \
    ::adobe::contract_violation::kind_t kind,                                 \
    const char *const file,                                                   \
    std::uint32_t const line,                                                 \
    const char *const message)                                                \
  {                                                                           \
    ::adobe::default_contract_violated(condition, kind, file, line, message); \
  }

#include <cstdlib>

#if defined(__clang__) || defined(__GNUC__) && __GNUC__ < 10
#define INTERNAL_ADOBE_BUILTIN_TRAP() __builtin_trap()
#elif defined(_MSC_VER)
#define INTERNAL_ADOBE_BUILTIN_TRAP() __debugbreak()
#else
#define INTERNAL_ADOBE_BUILTIN_TRAP() std::abort()
#endif

// Injects a definition of ::adobe::contract_violated that stops the
// program in the most efficient known way, without any diagnostic
// output.
#define ADOBE_MINIMAL_CONTRACT_VIOLATION_HANDLER()                \
  [[noreturn]] void ::adobe::contract_violated(const char *const, \
    ::adobe::contract_violation::kind_t,                          \
    const char *const,                                            \
    std::uint32_t const,                                          \
    const char *const)                                            \
  {                                                               \
    INTERNAL_ADOBE_BUILTIN_TRAP();                                \
  }

// Contract checking macros take a condition and an optional second argument.
//
// Information on how to simulate optional arguments is here:
// https://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros.
//
// The user experience when zero or three arguments are passed could
// be improved; portably detecting empty macro arguments could be used
// to help.

// Expands to its third argument
#define ADOBE_THIRD_ARGUMENT(arg0, arg1, invocation, ...) invocation

// ADOBE_PRECONDITION(<condition>);
// ADOBE_PRECONDITION(<condition>, <message: const char*>);
//
// Expands to a statement that reports a precondition failure (and
// <message> if supplied) when <condition> is false.
#define ADOBE_PRECONDITION(...)                                                          \
  ADOBE_THIRD_ARGUMENT(__VA_ARGS__, ADOBE_PRECONDITION_2, ADOBE_PRECONDITION_1, ignored) \
  (__VA_ARGS__)

// Expands to a statement that reports a precondition failure when
// condition is false.
#define ADOBE_PRECONDITION_1(condition)                         \
  if (condition)                                                \
    ;                                                           \
  else                                                          \
    ADOBE_CONTRACT_VIOLATION_LIKELIHOOD                         \
                                                                \
  ::adobe::contract_violated(#condition,                        \
    ::adobe::contract_violation::predefined_kind::precondition, \
    __FILE__,                                                   \
    __LINE__,                                                   \
    "")

// Expands to a statement that reports a precondition failure and
// <message: const char*> when condition is false.
#define ADOBE_PRECONDITION_2(condition, message)                \
  if (condition)                                                \
    ;                                                           \
  else                                                          \
    ADOBE_CONTRACT_VIOLATION_LIKELIHOOD                         \
                                                                \
  ::adobe::contract_violated(#condition,                        \
    ::adobe::contract_violation::predefined_kind::precondition, \
    __FILE__,                                                   \
    __LINE__,                                                   \
    message)

#define ADOBE_POSTCONDITION(condition)
#define ADOBE_INVARIANT(condition)

#endif
