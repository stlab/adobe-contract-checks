#ifndef ADOBE_CONTRACT_CHECKS_HPP
#define ADOBE_CONTRACT_CHECKS_HPP
#include <exception>
#include <stdexcept>
#include <system_error>

namespace adobe {

  class contract_violation final: public ::std::logic_error {
  private:
    std::error_code _code;
  public:
    explicit contract_violation(const char *message) : ::std::logic_error(message) {}

    [[nodiscard]] const std::error_code& code() const noexcept {
      return _code;
    }
  };

  [[noreturn]] void contract_violated(contract_violation const& reason);

  [[noreturn]] inline void default_contract_violated(contract_violation const& reason) {
    // This pattern, calling terminate while unwinding, causes most
    // standard libraries to report the exception that was thrown via
    // a default terminate handler.
    try {
      throw reason;
    } catch(...) {
      std::terminate();
    }
  }
}

// TODO: Supply a default terminate handler that calls
// get_current_exception and reports info.

#define ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER() \
  [[noreturn]] void ::adobe::contract_violated(::adobe::contract_violation const& reason) \
  { ::adobe::default_contract_violated(reason); }

#define ADOBE_PRECONDITION(condition) \
  if(condition); else \
    ::adobe::contract_violated(::adobe::contract_violation{"precondition"})

#define ADOBE_POSTCONDITION(condition)
#define ADOBE_INVARIANT(condition)

#endif
