#ifndef ADOBE_CONTRACT_CHECKS_HPP
#define ADOBE_CONTRACT_CHECKS_HPP
#include <exception>
#include <stdexcept>
#include <system_error>

namespace adobe {

  class contract_violation final: public ::std::logic_error {
  private:
    const std::error_code _code;
  public:
    contract_violation(const char* message) : ::std::logic_error(message) {}

    const std::error_code& code() const noexcept {
      return _code;
    }
  };

  [[noreturn]] void contract_violated(contract_violation const& reason);
}

#define ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER() \
  [[noreturn]] void ::adobe::contract_violated(::adobe::contract_violation const& reason) { \
  try { throw reason; } catch(...) { std::terminate(); } \
  }



#define ADOBE_PRECONDITION(condition) if(condition); else ::adobe::contract_violated(::adobe::contract_violation{"precondition"})

#define ADOBE_POSTCONDITION(condition)
#define ADOBE_INVARIANT(condition)

#endif
