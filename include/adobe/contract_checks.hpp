#ifndef ADOBE_CONTRACT_CHECKS_HPP
#define ADOBE_CONTRACT_CHECKS_HPP
#include <exception>
#include <stdexcept>
#include <system_error>

namespace adobe {

// The predefined kinds of contract violations provided by this library.
enum contract_violation_kind : int {
  precondition = 1,
  postcondition,
  invariant,
  safety_precondition,
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
  std::error_condition _condition;

public:
  explicit contract_violation(std::error_condition condition, const char *message)
    : ::std::logic_error(message), _condition(condition)
  {}

  [[nodiscard]] const std::error_condition &condition() const noexcept { return _condition; }
};

// The handler for contract violations, defined in the client's code.
//
// A default handler can be injected using ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER().
[[noreturn]] void contract_violated(contract_violation const &reason);

[[noreturn]] inline void default_contract_violated(contract_violation const &reason)
{
  // This pattern, calling terminate while unwinding, causes most
  // standard libraries to report the exception that was thrown via
  // a default terminate handler.
  try {
    throw reason;
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
      return "Precondition check violated";
    case contract_violation_kind::postcondition:
      return "Postcondition check violated";
    case contract_violation_kind::invariant:
      return "Invariant check violated";
    case contract_violation_kind::safety_precondition:
      return "Precondition check required to ensure safety violated";
    case contract_violation_kind::unconditional_fatal_error:
      return "Unconditional fatal error occurred";
    default:
      contract_violated(
        contract_violation{ contract_violation_kind::precondition, "unkown category kind" });
    }
  }

}// namespace detail
}// namespace adobe


// TODO: Supply a default terminate handler that calls
// get_current_exception and reports info.

#define ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()                                        \
  [[noreturn]] void ::adobe::contract_violated(::adobe::contract_violation const &reason) \
  {                                                                                       \
    ::adobe::default_contract_violated(reason);                                           \
  }

#define ADOBE_PRECONDITION(condition)                       \
  if (condition)                                            \
    ;                                                       \
  else                                                      \
    ::adobe::contract_violated(::adobe::contract_violation{ \
      ::adobe::contract_violation_kind::precondition, "precondition" })

#define ADOBE_POSTCONDITION(condition)
#define ADOBE_INVARIANT(condition)

#endif
