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

### Defining a contract violation handler

## Recommendations

- Use a contract violation handler (`adobe::contract_violated`) that
  unconditionally terminates the program (rationale: see [About
  Defensive Programming](#about-defensive-programming)).

- If your program needs to take emergency shutdown measures before
  termination, put those in a [terminate
  handler](https://en.cppreference.com/w/cpp/error/terminate_handler)
  that eventually calls
  [`std::abort()`](https://en.cppreference.com/w/cpp/utility/program/abort),
  and have your contract violation handler call
  [`std::terminate()`](https://en.cppreference.com/w/cpp/error/terminate).

     ```c++
     [[noreturn]] emergency_shutdown
     [[noreturn]] void ::adobe::contract_violated(
       const char *const condition,
       ::adobe::contract_violation::kind_t kind,
       const char *const file,
       std::uint32_t const line,
       const char *const message) noexcept
     {
       // ...whatever you want here...
       std::terminate();
     }
     ```

The predefined contract violation handlers provided by this library
follow the recommendations above.



## Rationale

### About Defensive Programming

According to [Wikipedia](https://en.wikipedia.org/wiki/Defensive_programming):

> **Defensive programming** is a form of defensive design intended to
> develop programs that are capable of detecting potential security
> abnormalities and make predetermined responses.[1] It ensures the
> continuing function of a piece of software under unforeseen
> circumstances.

In principle, defensive programming is a good idea.  In practice,
though, “unforeseen circumstances” usually mean the discovery of a bug
at runtime. Trying to keep running in the presence of bugs is in
general a losing battle:

- Code is littered with checks that obscure the logic of the program.
- The code paths that attempt to recover from bugs:
  - increase program size.
  - are almost never tested.
  - are often incorrect.
  - have unpredictable results, including data loss and security vulnerability.
  - create an infinite regression of self-checking, because in theory
    they could be buggy themselves.
- Bugs go undetected so code quality suffers.
- Even if bugs are logged:
  - they are often deprioritized because they are not crashes, so
    code quality suffers.
  - developers are robbed of any chance to get a debuggable program
    image; debugging becomes much harder, so code quality suffers.
- Nobody can actually think through the implications of everything
  being potentially buggy, so code beomes hard to reason about.
  Maintenance is more error-prone and quality suffers.

Note that in an unsafe language like C++, a seemingly recoverable
condition like the discovery of a negative index can easily be the
result of undefined behavior that also scrambled memory or causes
“impossible” execution.

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
