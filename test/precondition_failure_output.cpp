#include "adobe/contract_checks.hpp"

ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()

int main()
{
#line 9999
  ADOBE_PRECONDITION(1 < 0);
}
