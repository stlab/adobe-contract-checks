#ifndef ADOBE_CONTRACT_CHECKS_HPP
#define ADOBE_CONTRACT_CHECKS_HPP
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <system_error>

namespace adobe {

// The predefined kinds of contract violations provided by this library.
enum contract_violation_kind : int {
  precondition = 1,
  // A precondition check, that is required to prevent undefined behavior, failed.
  safety_precondition,
  postcondition,
  invariant,
  unconditional_fatal_error
};

namespace detail {
  // Implementation of contract_violation_category::message.
  inline std::string contract_violation_category_message(int kind);

  // The domain of error_condition values for contract violations.
  class contract_violation_category final : public std::error_category
  {
  public:
    // Returns the name of this category.
    [[nodiscard]] const char *name() const noexcept override { return "Contract violation"; }

    // Returns a human readable description of the violation.
    [[nodiscard]] std::string message(int kind) const override
    {
      return detail::contract_violation_category_message(kind);
    }
  };

}// namespace detail

inline const std::error_category &contract_violation_category()
{
  static const detail::contract_violation_category instance;
  return instance;
}
}// namespace adobe

// ------------------------------------------------------------------------------------------
// These two definitions together make contract_violation_kind implicitly
// convertible to error_condition.
//
namespace adobe {
inline std::error_condition make_error_condition(contract_violation_kind e)
{
  return std::error_condition(static_cast<int>(e), contract_violation_category());
}

}// namespace adobe

namespace std {
template<> struct is_error_condition_enum<adobe::contract_violation_kind> : public true_type
{
};
}// namespace std
// ------------------------------------------------------------------------------------------

namespace adobe {

// A violation of some API contract.
class contract_violation final : public ::std::logic_error
{
private:
  // The kind of violation.
  std::error_condition _condition;

  // The file in which the violation occurred.
  const char *_file = "";

  // The line number on which the violation occurred.
  std::uint32_t _line = 0;

public:
  explicit contract_violation(std::error_condition condition,
    const char *file,
    std::uint32_t line,
    const char *message)
    : ::std::logic_error(message), _condition(condition), _file(file), _line(line)
  {}

  // Returns the file in which the violation occurred.
  const char *file() const { return _file; }

  // Returns the line number on which the violation occurred.
  std::uint32_t line() const { return _line; }

  const std::error_condition &condition() const noexcept { return _condition; }

  void print_report() const
  {
    std::fprintf(stderr, "%s:%d: %s: %s\n", _file, _line, _condition.message().c_str(), what());
  }
};

// The handler for contract violations, defined in the client's code.
//
// A default handler can be injected using ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER().
[[noreturn]] void contract_violated(contract_violation_kind condition,
  const char *file,
  std::uint32_t line,
  const char *message);

[[noreturn]] inline void default_contract_violated(contract_violation const &reason)
{
  // This pattern, calling terminate while unwinding, causes most
  // standard libraries to report the exception that was thrown via
  // a default terminate handler.
  try {
    throw reason;
  } catch (adobe::contract_violation const &e) {
    e.print_report();
    std::terminate();
  } catch (...) {
    std::terminate();
  }
}

namespace detail {
  // Implementation of contract_violation_category::message.
  inline std::string contract_violation_category_message(int kind)
  {
    switch (kind) {
    case contract_violation_kind::precondition:
    case contract_violation_kind::safety_precondition:
      return "Precondition violated";
    case contract_violation_kind::postcondition:
      return "Postcondition violated";
    case contract_violation_kind::invariant:
      return "Invariant violated";
    case contract_violation_kind::unconditional_fatal_error:
      return "Unconditional fatal error";
    default:
      contract_violated(
        contract_violation_kind::precondition, __FILE__, __LINE__, "unkown category kind");
    }
  }

}// namespace detail
}// namespace adobe


// TODO: Supply a default terminate handler that calls
// get_current_exception and reports info.

#define ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()                                         \
  [[noreturn]] void ::adobe::contract_violated(::adobe::contract_violation_kind condition, \
    const char *file,                                                                      \
    std::uint32_t line,                                                                    \
    const char *message)                                                                   \
  {                                                                                        \
    ::adobe::default_contract_violated(                                                    \
      ::adobe::contract_violation(condition, file, line, message));                        \
  }

// Optional macro arguments:
// https://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros

#define ADOBE_PRECONDITION_X(arg0, arg1, invocation, ...) invocation

#define ADOBE_PRECONDITION(...) \
  ADOBE_PRECONDITION_X(         \
    __VA_ARGS__, ADOBE_PRECONDITION_2(__VA_ARGS__), ADOBE_PRECONDITION_1(__VA_ARGS__))

#define ADOBE_PRECONDITION_1(condition) \
  if (condition)                        \
    ;                                   \
  else                                  \
    ::adobe::contract_violated(         \
      ::adobe::contract_violation_kind::precondition, __FILE__, __LINE__, "precondition")

#define ADOBE_PRECONDITION_2(condition, message) \
  if (condition)                                 \
    ;                                            \
  else                                           \
    ::adobe::contract_violated(                  \
      ::adobe::contract_violation_kind::precondition, __FILE__, __LINE__, message)

#define ADOBE_POSTCONDITION(condition)
#define ADOBE_INVARIANT(condition)

#endif
