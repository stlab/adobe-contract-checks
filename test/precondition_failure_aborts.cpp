#include "adobe/contract_checks.hpp"

ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()

int main() { ADOBE_PRECONDITION(false); }
