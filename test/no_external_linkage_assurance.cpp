// This file merely ensures that the header doesn't define any symbols with external linkage by
// including it in a second translation unit.  The program should fail to link in that case.
#include "adobe/contract_checks.hpp"// NOLINT
