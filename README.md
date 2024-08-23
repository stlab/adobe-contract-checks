[![ci](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml)
[![CodeQL](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml)

------

# Adobe Contract Checking

This library is for checking that software
[contracts](https://en.wikipedia.org/wiki/Design_by_contract) are
upheld.  In C++ these checks can be especially important for safety
because failure to satisfy contracts typically leads to [undefined
behavior](https://en.wikipedia.org/wiki/Undefined_behavior), which can
manifest as crashes, data loss, and security vulnerabilities.  This
library largely improves upon the standard
[`assert`](https://en.cppreference.com/w/cpp/error/assert) macro.

## Design by Contract

[Design by Contract](https://en.wikipedia.org/wiki/Design_by_contract)
is *the* industry-standard way to describe the requirements and
guarantees of any software component.  It is based on three concepts:

- **Preconditions**: what the caller of a function must ensure for the
  function to behave as documented.  A precondition violation
  indicates a bug in the caller.

- **Postconditions** describe a function's side-effects and return
  value. Postconditions need not be upheld if the function reports an
  error (such as memory exhaustion), or if preconditions were
  violated.  Otherwise, a postcondition violation indicates
  a bug in the callee.

- **Invariants**: conditions that always hold at some point in the
  code.  The most common kind of invariants are **class invariants**,
  which hold at any point where an instance can be inspected from
  outside the class, but other invariant checks (especially loop
  invariants) are also useful.

This library provides macros for cheking preconditions
(`ADOBE_PRECONDITION`) and invariants (`ADOBE_INVARIANT`).
Postconditions should be checked by unit tests
([rationale](#why-this-library-provides-no-postcondition-check)).
When these precondition or invariant checks fail, the program is
terminated ([rationale](#about-defensive-programming)).

### Documenting Contracts

A function's specification must at least describe its preconditions
and postconditions. The specification of a class must describe its
publicly-visible invariants.  The minimal documentation required for
any component is its contract, and usually, a short sentence fragment
is sufficient to convey all the important information.

Additionally describing these conditions in code and checking them at
runtime can be  a powerful way to catch bugs early and prevent their
damaging effects.

That said, documentation is the primary vehicle for expressing
contracts for two reasons:

1. not all contracts can be checked at runtime, e.g. “`p` points to a
   valid object,” or “the `callback` parameter always returns a value
   from `0` through `1.0`.”
2. Reasoning locally about code depends on being able to understand
   the contract of each component the code uses without looking at the
   component's implementation.

#### How Reported Errors Fit In

The condition that causes a function to throw an exception or
otherwise report an error to its caller should not be treated as a
precondition.  Instead, make the error reporting behavior part of the
function's specification: document the behavior and test it to make
sure that it works.  Also, do not describe the error report as part of
the postcondition. **Reporting an error to the caller exempts a
function from fulfilling postconditions** and can be thought of as an
allowed postcondition failure.

For example:

```c++
// Returns a pointer to a colorful widget.
//
// Throws std::bad_alloc if memory is exhausted.
std::unique_ptr<Widget> build_widget();
```

The first line of documentation above describes the function's
postcondition.  The second line describes its error reporting,
separately from the postcondition.  You can eliminate the need to
document exceptions by setting a project-wide policy that, unless a
function is `noexcept`, it can throw anything.  Another way is by
encoding the ability to return an error in the function's
signature. Documenting which exceptions can be thrown or errors
reported is not crucial, but documenting the fact that an error can
occur is.

Unless otherwise specified in the function's documentation, a reported
error means all objets the function would otherwise modify are invalid
for all uses except as the target of destruction or assignment.

## Basic C++ Usage

This is a header-only library.  To use it from C++, simply put the
`include` directory of this repository in your `#include` path, and
`#include <adobe/contract_checks.hpp>`.

```c++
#include <adobe/contract_checks.hpp>
```

The two macros used to check contracts,`ADOBE_PRECONDITION` and
`ADOBE_INVARIANT`, eachtake one required argument and one optional
argument:

- `condition` (required): an expression convertible to `bool`; if `false`, the
  violation handler is invoked.
- `message` (optional): an expression convertible to `const char*`
  pointing to a
  [null-terminated](https://en.cppreference.com/w/cpp/string/byte)
  message that is additionally passed to the violation handler. The
  default `message` value is the empty string, `""`.

For example,

```c++
#include <adobe/contract_checks.hpp>
#include <climits>

// A half-open range of integers.
// - Invariant: start() <= end().
class int_range {
  // The lower bound; if the range is non-empty, its
  // least contained value.
  int _start;
  // The upper bound; if the range is non-empty, one
  // greater than its greatest contained value.
  int _end;

  // Fails a contract check if the invariants of `*self` are violated.
  void check_invariant() const { ADOBE_INVARIANT(start() <= end()); }
public:
  // An instance with the given bounds.
  // Precondition: end >= start
  int_range(int start, int end) : _start(start), _end(end) {
    ADOBE_PRECONDITION(end >= start, "invalid range bounds.");
    check_invariant();
  }

  // Returns the lower bound: if *this is non-empty, its
  // least contained value.
  int start() const { return _start; }

  // Returns the upper bound; if *this is non-empty, one
  // greater than its greatest contained value.
  int end() const { return _end; }

  // Increases the upper bound by 1.
  // Precondition: end() < INT_MAX
  void grow_rightward() {
    ADOBE_PRECONDITION(end() < INT_MAX);
    int old_end = end();
    _end += 1;
    check_invariant();
  }

  // more methods...
};
```

## Basic CMake Usage

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

  add_library(my-library my-library.cpp)
  target_link_libraries(my-library PRIVATE adobe-contract-checks)

  add_executable(my-executable my-executable.cpp)
  target_link_libraries(my-executable PRIVATE adobe-contract-checks)
  ```

## Recommendations

-  Start by checking whatever you can, and worry about performance
   later. Checks are often critical for safety. [Configuration
   options](#configuration) can be used to mitigate or eliminate costs
   later if necessary.

-  If you have to prioritize, precondition checks are the most
   important; they are your last line of defense against undefined
   behavior.

   Class invariant checks can often give you more bang for your buck,
   though, because they can be used to eliminate the need for
   precondition checks and verbose documentation across many
   functions.

   ```c++
   // Returns the day of the week corresponding to the date described
   // by "<year>-<month>-<day>" (interpreted in ISO standard date
   // format).
   //
   // Precondition: "<year>-<month>-<day>" is a valid ISO standard date.
   day_of_the_week day(int year, int month, int day) {
     ADOBE_PRECONDITION(is_valid_date(year, month, day));
     // implementation starts here.
   }

   // ------- vs -------

   // Returns the day of the week corresponding to `d`.
   day_of_the_week day(date d) {
     // implementation starts here.
   }
   ```

   The second function above benefits by accepting a `date` type whose
   invariant ensures its validity.

- The conditions in your checks should not have side-effects that
  change program behavior; readers expect to be able to skip over
  these checks when reasoning about code.

- Group all precondition checks immediately after a function's
  opening brace, and don't allow any code to sneak in before them.

- Give your `struct` or `class` a `void check_invariant() const`
  method containing `ADOBE_INVARIANT` invocations, so that invariant
  checking can be centralized.  Invoke it from each public mutating
  friend or member function just before each `return` or before
  `*this` becomes visible to any other component such as a callback
  parameter.

- If your program needs to take emergency shutdown measures before
   termination, put those in a [terminate
   handler](https://en.cppreference.com/w/cpp/error/terminate_handler)
   that eventually calls
   [`std::abort()`](https://en.cppreference.com/w/cpp/utility/program/abort).

     ```c++
     #include <cstdlib>
     #include <exception>

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
     ...
     ```

   That way, other reasons for sudden termination, such as
   uncaught exceptions, will still cause emergency shutdown to execute.

- Don't disable critical checks in shipping code unless a measurable
  unacceptable performance cost is found, and after assessing the risk
  of undefined behavior should the check be skipped.  In that case,
  disable the expensive checks selectively. For example, if you have
  `NDEBUG` defined in your release build, you can enable the check for
  debug builds only.

   ```
   #ifndef NDEBUG // too expensive for release
   ADOBE_PRECONDITION(some_expensive_call());
   #endif
   ```

## Rationales

### Why This Library Provides No Postcondition Check

Checking postconditions is practically the entire raison d'être of
unit tests, and many good frameworks for unit testing exist.  Adding a
postcondition check to this library would just create confusion about
where postcondition checks should go and the true purpose of unit
testing. Also postcondition checks for most mutating functions need to
makes an initial copy of everything being mutated, which can be
prohibitively expensive even for debug builds.

### About Defensive Programming

According to
[Wikipedia](https://en.wikipedia.org/wiki/Defensive_programming):

> **Defensive programming** is a form of defensive design intended to
> develop programs that are capable of detecting potential security
> abnormalities and make predetermined responses.[1] It ensures the
> continuing function of a piece of software under unforeseen
> circumstances.

In principle, defensive programming as defined above is a good idea.
As defensive programming is commonly practiced, though, “unforeseen
circumstances” usually mean the discovery of a bug at runtime. Trying
to keep running in the presence of bugs is in general a losing battle:

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

The usual procedures for development with cmake apply.  One typical
set of commands might be:

```sh
cmake -Wno-dev -S . -B ../build -GNinja        # configure
cmake --build ../build                         # build/rebuild after changes
ctest --output-on-failure --test-dir ../build  # test
```

## Reference

### Configuration

The behavior of this library is configured by one preprocessor symbol,
`ADOBE_CONTRACT_VIOLATION`.  It can have one of three definitions, or
be left undefined.

- `verbose`: as much information as possible is collected from the
  site of a detected contract violation and reported to the standard
  error stream before `std::terminate()` is invoked.  This behavior is
  also the default if `ADOBE_CONTRACT_VIOLATION` is left undefined.

- `minimal`: When a contract violation is detected, `std::terminate()`
  is invoked immediately.  Aside from code to check the condition and
  call `terminate`, none of the arguments to a contract checking macro
  generates any code or data.

- `unsafe`: Contract checking macros have no effect and generate no
  code or data.  Not recommended for general use, but can be useful
  for measuring the overall performance impact of checking in a
  program.

This library can only have one configuration in an executable, so the
privilege of choosing a configuration for all components always
belongs to the top-level project in a build.

To avoid ODR violations, any binary libraries (not built from source)
that use this library must use the same version of this library, and
if they use this library in public header files, must have been built
with the same value of `ADOBE_CONTRACT_VIOLATION`.

In CMake you could use a pattern like this:

```cmake
if(PROJECT_IS_TOP_LEVEL)
  # Set adobe-contract-checks configuration default based on build
  # type.
  if(CMAKE_BUILD_TYPE EQUALS "Debug")
    set(default_ADOBE_CONTRACT_VIOLATION "verbose")
  else()
    set(default_ADOBE_CONTRACT_VIOLATION "minimal")
  endif()
  # declare the option so user can configure on CMake command-line or
  # in CMakeCache.txt.
  option(ADOBE_CONTRACT_VIOLATION
    "Behavior when a contract violation is detected"
    "${default_ADOBE_CONTRACT_VIOLATION}")
endif()

# add the preprocessor definition to the C++ compiler command line for
# all targets.
if(DEFINED ADOBE_CONTRACT_VIOLATION)
  add_compile_definitions("ADOBE_CONTRACT_VIOLATION=${ADOBE_CONTRACT_VIOLATION}")
endif()
```
