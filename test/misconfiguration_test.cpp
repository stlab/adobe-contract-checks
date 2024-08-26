#define ADOBE_CONTRACT_VIOLATION misconfigured
// CI somehow escapes quotes in static assertion strings, so we can't check for those below.

// build error: Unknown configuration ADOBE_CONTRACT_VIOLATION=misconfigured.  Valid values are
#include "adobe/contract_checks.hpp"
