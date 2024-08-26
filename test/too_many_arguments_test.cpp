#include "adobe/contract_checks.hpp"

int main()
{
  // It's hard to pass too many arguments without detection...

  // build error:too_many_arguments_test.cpp
  ADOBE_PRECONDITION(true, "message", 1);

  // ...but see inadequate_arity_checking_tests.cpp
}
