# Adobe Contract Checking

This library is for checking that software
[contracts](https://en.wikipedia.org/wiki/Design_by_contract) are
upheld.  In C++ these checks can be especially important for safety
because failure to satisfy contracts typically leads to [undefined
behavior](https://en.wikipedia.org/wiki/Undefined_behavior), which can
manifest as crashes, data loss, and security vulnerabilities.

[Design by Contract](https://en.wikipedia.org/wiki/Design_by_contract)
is *the* industry-standard way to describe the requirements and
guarantees of any software component.  It is based on three concepts:

- **Precondition**: what the caller of a function must ensure for the
  function to behave as documented.  A precondition violation
  indicates a bug in the caller, and in that case the call in
  principle do anything.

- **Postcondition**: a function's side-effects and return
  value. Postconditions need not be upheld if the function reports an
  error (such as memory exhaustion), or if preconditions were
  violated.  Otherwise, a failure to satisfy postconditions indicates
  a bug in the callee.

- **Invariants**: conditions that always hold at some point in the
  code.  The most common and useful kind of invariants are **class
  invariants**, which hold at any point where an instance can be
  inspected from outside the class.

Something about how these go in documentation and this library
provides corresponding checking macros to check them.

## Usage

- This is a header-only library.  To use it from C++, simply put the
  `include` directory of this repository in your `#include` path, and
  `#include <adobe/contract_checks.hpp>`.

- Example here

- To use this library from CMake, follow this pattern:

  ```cmake
  include(FetchContent)
  if(PROJECT_IS_TOP_LEVEL)
    FetchContent_Declare(
      adobe-contract-checks
      GIT_REPOSITORY https://github.com/stlab/adobe-contract-checks.git
      GIT_TAG        <this library's release version>
    )
    FetchContent_MakeAvailable(adobe-contract-checks)
  endif()
  find_package(adobe-contract-checks)

  add_library(my-library ...)
  target_link_libraries(my-library PRIVATE adobe-contract-checks)

  add_executable(my-executable ...)
  target_link_libraries(my-executable PRIVATE adobe-contract-checks)
  ```

### Defining a contract violation handler

## Recommendations

1. Use a contract violation handler (`adobe::contract_violated`) that
  unconditionally terminates the program (rationale: see [About
  Defensive Programming](#about-defensive-programming)). The
  predefined contract violation handlers provided by this library
  follow this recommendation.

2. Start by checking whatever you can, and worry about performance
   later. Checks are often critical for safety. [Configuration
   options](#configuration) can be used to mitigate or eliminate costs
   later if necessary.

4. The conditions in your checks should not have side-effects that
   change program behavior, because checks are sometimes turned off by
   configuration.

2. Group all precondition checks immediately after a function's
   opening brace, and don't allow any code to sneak in before them.

3. Group all postcondition checks just before your function returns.
   (That may mean temporarily storing a return value in a local
   variable so it can be tested.)

4. Give your `struct` or `class` a `bool invariant() const` method that containing `ADOBE_CONTRACT`

6. If a function throws exceptions or can otherwise report an error,
   don't make call that a precondition violation.  Instead, make that
   behavior part of the function's specification: document the
   conditions, the resulting behavior, and test it to make sure that
   it works.

2. If your program needs to take emergency shutdown measures before
   termination, put those in a [terminate
   handler](https://en.cppreference.com/w/cpp/error/terminate_handler)
   that eventually calls
   [`std::abort()`](https://en.cppreference.com/w/cpp/utility/program/abort),
   and have your contract violation handler call
   [`std::terminate()`](https://en.cppreference.com/w/cpp/error/terminate).

     ```c++
     #include <cstdint>
     #include <cstdlib>
     #include <exception>
     #include <iostream>

     [[noreturn]] void emergency_shutdown() noexcept;

     const std::terminate_handler previous_terminate_handler
       = std::set_terminate(emergency_shutdown);

     [[noreturn]] void emergency_shutdown() noexcept
     {
       // emergency shutdown measures here.

       if (previous_terminate_handler != nullptr)
         { previous_terminate_handler(); }
       std::abort();
     }

     [[noreturn]] void ::adobe::contract_violated(
       const char *const condition,
       ::adobe::contract_violation::kind_t kind,
       const char *const file,
       std::uint32_t const line,
       const char *const message)
     {
       // whatever you want here.
       std::terminate();
     }
     ```

   That way, other reasons for unexpected termination such as uncaught
   exceptions, will still cause emergency shutdown.

3. If your custom contract violation handler needs to print a
  description of the failure, use [Gnu standard error
  format](https://www.gnu.org/prep/standards/html_node/Errors.html#Errors),
  which  will be automatically understood by many tools.  The
  following expression will print such a report to the standard error
  stream:

      ```c++
      adobe::contract_violation(
        condition, kind, file, line, message).print_report();
      ```

5. Don't disable critical checks in shipping code unless an measurable
   unacceptable performance cost is found.  In that case, disable the
   expensive checks selectively, e.g.

   ```
   #ifndef NDEBUG // too expensive for release
   ADOBE_PRECONDITION(some_expensive_call());
   #endif
   ```

## Rationale

### Performance tuning and configuration complexity

Unfortunately contract checks have some performance cost. If
programmers fear that writing a check will bake that cost into their
code, they are likely to skip writing the check altogether.  To
mitigate that effect, we supply extensive
[configuration](#configuration) options that allowing projects to tune
the overheads incurred by checking, _after checks have already been
written_.

We want programmers to write checks freely; even if for some reason
checks have to be disabled during regular development, they help to
document code and can be turned on temporarily to help track down
difficult bugs.

### About Defensive Programming

According to
[Wikipedia](https://en.wikipedia.org/wiki/Defensive_programming):

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

### Configuration

The behavior of this library is configured by preprocessor symbols.

Instead of passing many `-D` options in your compiler command-line,
you can put the definition of these symbols in a header file and
simply define `ADOBE_CONTRACT_CHECKS_CONFIGURATION` as the `#include`
argument to that file:

```sh
c++ -D \
    -I .\
    ADOBE_CONTRACT_CHECKS_CONFIGURATION="<myproject/adobe_contract_checks_config.hpp>" \
    myproject_source.cpp -o myproject_executable
```

If you are using this project via CMake, defining the same symbol in
CMake, on the command line or in your will cause all clients of this
library in your build to use that configuration file.

This library can only have one configuration in an executable, so the
privilege of configuring it belongs to the executable.  In CMake,

```cmake
if(PROJECT_IS_TOP_LEVEL)
  set(ADOBE_CONTRACT_CHECKS_CONFIGURATION "<myproject/adobe_contract_checks_config.hpp>")
endif()
```

#### Preprocessor configuration symbols

##### Contract handler definition

- `ADOBE_CONTRACT_VIOLATED_RETURNS`: define this symbol if your
  contract violation handler, against our advice (see [recommendation
  1](#recommendations)), can return to its caller.

- `ADOBE_CONTRACT_VIOLATED_THROWS`: define this symbol if your
  contract violation handler, against our advice (see [recommendation
  1](#recommendations)), can throw exceptions.

- `ADOBE_CONTRACT_VIOLATED_INLINE_BODY`: if you want the contract
  violation handler to be defined inline, make this symbol expand to
  its body.  When the body is lighter-weight than a call to the
  handler would be, an inline handler can limit the code generated at
  the use site. For example,

  ```c++
  #define ADOBE_CONTRACT_VIOLATED_INLINE_BODY { ADOBE_BUILTIN_TRAP(); }
  ```

  You might use this option in release builds when a minimal handler
  is required.  **Note:** Defining a more complex handler inline
  usually will increase binary sizes and may hurt performance.

##### Symbols that minimize generated code and data

- `ADOBE_NO_CONTRACT_CONDITION_STRINGS`: define this symbol to
  suppress the generation of strings describing failed check
  conditions.  The empty string will be used instead.

- `ADOBE_NO_CONTRACT_FILENAME_STRINGS`: define this symbol to suppress
  the generation of strings describing the file in which failed checks
  occurred.  `"<unknown file>"` will be used instead.

- `ADOBE_SKIP_NONCRITICAL_PRECONDITION_CHECKS`: define this symbol to
  make uses of `ADOBE_NONCRITICAL_PRECONDITION` generate no code.

- `ADOBE_SKIP_ALL_CONTRACT_CHECKS`: define this symbol to make all
  contract checking macros generate no code.  Not recommended for
  general use, but can be useful for measuring the overall performance
  impact of checking in a program.

------------------

[![ci](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml)
[![CodeQL](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml)


## More Details

 * [Dependency Setup](README_dependencies.md)
 * [Building Details](README_building.md)
 * [Troubleshooting](README_troubleshooting.md)
 * [Docker](README_docker.md)
