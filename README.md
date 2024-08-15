# Adobe Contract Checking

This is a library of macros used to check that software
[contracts](https://en.wikipedia.org/wiki/Design_by_contract) are
upheld.  In C++ these checks can be especially important for safety
because failure to satisfy contracts typically leads to [undefined
behavior](https://en.wikipedia.org/wiki/Undefined_behavior), which can
manifest as crashes, data loss, and security vulnerabilities.

[Design by Contract](https://en.wikipedia.org/wiki/Design_by_contract)
is *the* industry-standard way to describe the requirements and
guarantees of any software component.  It is based on three concepts:

- **Preconditions**: what the caller of a function must ensure for the
  function to behave as documented.
- **Postconditions**: what a function promises to its caller if
  preconditions are satisfied if it returns normally without reporting
  an error.
- **Class invariants**: conditions that hold whenever an instance is
  inspected from outside the class.

If the function is documented as
  reporting errors under certain conditions, those conditions are not
  preconditions.

  Preconditions do not include
  conditions under which a function is specified to report errors.

If preconditions are not satisfied under which the function behaves
  as documented.  A function

## Usage

- CMake option to configure how this library is used generates
  appropriate usage requirement.

## Recommendations


## Rationale

## Development

Building/Running Tests

## Reference


------------------

[![ci](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml)
[![CodeQL](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml)


## More Details

 * [Dependency Setup](README_dependencies.md)
 * [Building Details](README_building.md)
 * [Troubleshooting](README_troubleshooting.md)
 * [Docker](README_docker.md)
