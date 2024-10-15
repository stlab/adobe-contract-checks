[![ci](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml)
[![CodeQL](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml)

---

# Adobe Contract Checking

This library is for checking that software
[contracts](https://en.wikipedia.org/wiki/Design_by_contract) are
upheld. In C++ these checks can be especially important for safety
because failure to satisfy contracts typically leads to [undefined
behavior](https://en.wikipedia.org/wiki/Undefined_behavior), which can
manifest as crashes, data loss, and security vulnerabilities.

This library provides [macros](#basic-c-usage) for checking preconditions and
invariants, and can be viewed as an improvement upon the
standard [`assert`](https://en.cppreference.com/w/cpp/error/assert)
macro. However, the discipline and rationales documented here are
just as important to the library's value as are its mechanics.

## Design by Contract

[Design by Contract](https://en.wikipedia.org/wiki/Design_by_contract)
is _the_ industry-standard way to describe the requirements and
guarantees of any software component. It is based on three concepts:

- **Preconditions**: what the caller of a function must ensure for the
  function to behave as documented. A precondition violation
  indicates a bug in the caller.

- **Postconditions** describe a function's side-effects and return
  value. Postconditions need not be upheld if the function reports an
  error (such as memory exhaustion), or if preconditions were
  violated. Otherwise, a postcondition violation indicates
  a bug in the callee.

- **Invariants**: conditions that always hold at one or more specific
  points in the
  code. The most common kind of invariants are **class invariants**,
  which hold wherever it is valid to inspect an instance from
  outside the class, but other invariants (especially [loop
  invariants](https://en.wikipedia.org/wiki/Loop_invariant)) are also useful.

Postconditions should be checked by unit tests
([rationale](#why-this-library-provides-no-postcondition-check)).

### Documenting Contracts

**The minimal documentation required for any component is its contract**.
Writing this documentation need not be a burden; usually, a short sentence
fragment is sufficient ([examples](#basic-c-usage)).

- The contract of a function describes its preconditions, postconditions
  (which include the return value).
- The contract of a class describes its publicly-visible invariants.

**Documentation is the primary vehicle for expressing
contracts**

<details>
<summary markdown='span'><em>Rationale</em></summary>

1. Some contracts cannot be checked at runtime. For example, there's no
   way to check these preconditions:
   ```c++
   /// Returns the frobnication of `p` and `f`.
   ///
   /// - Precondition: `p` points to an initialized object.
   /// - Precondition: `f(x)` returns a value from `0` through `1.0`
   ///   for any `x`.”
   auto frob(X* p, float (*f)(int)) -> bool;
   ```
2. Reasoning locally about code depends on being able to understand
   the contract of each component the code uses without looking at the
   component's implementation. From a client's point of view, contract
   checks are hidden inside the implementation.

</details>

Additionally describing contracts in code and checking them at
runtime can be a powerful way to catch bugs early and prevent their
damaging effects. That's the role of this library.

### How Reported Errors Fit In

The condition that causes a function to throw an exception or
otherwise report an error to its caller should not be treated as a
precondition. Instead, make the error reporting behavior part of the
function's specification: document the behavior and test it to make
sure that it works. Also, do not describe the error report as part of
the postcondition. **Reporting an error to the caller exempts a
function from fulfilling postconditions** and can be thought of as an
unavoidable failure to fulfill postconditions.

For example:

```c++
/// Returns a pointer to a colorful widget.
///
/// Throws std::bad_alloc if memory is exhausted.
std::unique_ptr<Widget> build_widget();
```

The first line of documentation above describes the function's
postcondition. The second line describes its error reporting,
separately from the postcondition. You can eliminate the need to
document exceptions by setting a project-wide policy that, unless a
function is `noexcept`, it can throw anything. You can eliminate the
need to document returned errors by encoding the ability to return an
error in the function's signature. Documenting _which_ exceptions can be
thrown or errors reported is not crucial, but documenting the fact
_that_ an error can occur is.

Unless otherwise specified in the function's documentation, a reported
error means all objects the function would otherwise modify are
invalid for all uses, except as the target of destruction or
assignment. **Discarding this invalid data is the obligation of code
that stops error propagation to callers**.

Because this invalid data must be discarded, **code that reports or
propagates errors need not uphold class invariants**;
the only properties of the class that must be maintained are
destructibility and assignability. Note that this policy is less
strict than what is implied by the [basic exception safety
guarantee](https://en.wikipedia.org/wiki/Exception_safety#Classification),
and supersedes the stricter policy with the endorsement of its inventor.

Upholding the obligation to discard invalid mutated data is reasonably
easy if types under mutation have [value
semantics](https://www.jot.fm/issues/issue_2022_02/article2.pdf),
because data forms a tree and the invalidated data is always uniquely
a part of the objects being mutated at the level of the error-handling
code. Otherwise it may be necessary to discard other parts of the
object graph.

The usual, and most useful, way of specifying that data under mutation
is _not_ invalidated is by making the [strong
guarantee](https://en.wikipedia.org/wiki/Exception_safety#Classification)
that there are no effects in case of an error (where possible without loss
of efficiency). When the callee can make that promise, it exempts the
caller from discarding invalid data.

## Basic C++ Usage

This is a header-only library. To use it from C++, simply put the
`include` directory of this repository in your `#include` path, and
`#include <adobe/contract_checks.hpp>`.

```c++
#include <adobe/contract_checks.hpp>
```

The two macros used to check contracts,`ADOBE_PRECONDITION` and
`ADOBE_INVARIANT`, each take one required argument and one optional
argument:

- `condition` (required): an expression convertible to `bool`; if `true`,
  the contract was upheld and the macro has no effects.
- `message` (optional): an expression convertible to `const char*`
  pointing to a
  [null-terminated](https://en.cppreference.com/w/cpp/string/byte)
  message. The
  default `message` value is the empty string, `""`.

The precise effects of a contract violation depend on this library's
[configuration](#configuration).

For example,

```c++
#include <adobe/contract_checks.hpp>
#include <climits>

/// A half-open range of integers.
/// - Invariant: start() <= end().
class int_range {
  /// The lower bound; if `*this` is non-empty, its
  /// least contained value.
  int _start;
  /// The upper bound; if `*this` is non-empty, one
  /// greater than its greatest contained value.
  int _end;

  /// Returns `true` if and only if the invariants are intact.
  bool is_valid() const { return start() <= end(); }
public:
  /// An instance with the given bounds.
  /// - Precondition: `end >= start`.
  int_range(int start, int end) : _start(start), _end(end) {
    ADOBE_PRECONDITION(end >= start, "invalid range bounds.");
    ADOBE_INVARIANT(is_valid());
  }

  /// Returns the lower bound: if `*this` is non-empty, its
  /// least contained value.
  int start() const { return _start; }

  /// Returns the upper bound; if `*this` is non-empty, one
  /// greater than its greatest contained value.
  int end() const { return _end; }

  /// Increases the upper bound by 1.
  /// - Precondition: `end() < INT_MAX`.
  void grow_rightward() {
    ADOBE_PRECONDITION(end() < INT_MAX);
    int old_end = end();
    _end += 1;
    ADOBE_INVARIANT(is_valid());
  }

  /// more methods...
};
```

## Configuration

The behavior of this library is configured by one preprocessor symbol,
`ADOBE_CONTRACT_VIOLATION`. It can be defined to one of three
strings, or be left undefined, in which case it defaults to `verbose`.

- `ADOBE_CONTRACT_VIOLATION=verbose`: as much information as possible
  is collected from the site of a detected contract violation and
  reported to the standard error stream before `std::terminate()`
  is called.
- `ADOBE_CONTRACT_VIOLATION=lightweight`: When a contract violation is
  detected, `std::terminate()` is invoked. Aside from
  code to check the condition and call `terminate`, none of the
  arguments to a contract checking macro generates any code or data.

- `ADOBE_CONTRACT_VIOLATION=unsafe`: Contract checking macros have no
  effect and generate no code or data. Not recommended for general
  use, but can be useful for measuring the overall performance impact
  of checking in a program.

- `ADOBE_CONTRACT_VIOLATION=custom_verbose`: When a contract violation is detected, a custom handler
  is invoked. The client must define the handler at global scope with the signature:

  ```cpp
  [[noreturn]] void adobe_contract_violation_verbose(const char *condition,
    adobe::contract_violation_kind kind, const char *file, std::uint32_t line,
    const char *message) {
      // implementation
  }
  ```

  The parameters are a string representation of the failed condition, the contract violation kind
  (`precondition` or `invariant`), the file and line number of the failed check, and the
  message parameter to the failing check macro, or the empty string if no message was provided.
  The function should report the violation in a way that is appropriate for the
  application, and must not return to its caller.

- `ADOBE_CONTRACT_VIOLATION=custom_lightweight`: When a contract violation is detected, a custom
  handler is invoked. The client must define the handler at global scope with the signature:

  ```cpp
  [[noreturn]] void adobe_contract_violation_lightweight() {
      // implementation
  }
  ```

  The function should report the violation in a way that is appropriate for the application, and
  must not return to its caller.
  function must not return.

In `verbose` and `lightweight` modes, a failed check ultimately calls
[`std::terminate()`](https://en.cppreference.com/w/cpp/error/terminate)
because:

1. Continuing in the face of a detected bug is [considered
   harmful](#about-defensive-programming), and
2. Unlike other methods
   of halting, `std::terminate()` allows for [emergency shutdown
   measures](#emergency-shutdown).

This library can only have one configuration in an executable, so the
privilege of choosing a configuration for all components always
belongs to the top-level project in a build.

To avoid ODR violations, any binary libraries (not built from source)
that use this library must use the same version of this library, and
if they use this library in public header files, must have been built
with the same value of `ADOBE_CONTRACT_VIOLATION`.

## Basic CMake Usage

To use this library from CMake and uphold the discipline described
above, you might put something like this in your project's top level
`CMakeLists.txt`:

```cmake
include(FetchContent)
if(PROJECT_IS_TOP_LEVEL)
  FetchContent_Declare(
    adobe-contract-checks
    GIT_REPOSITORY https://github.com/stlab/adobe-contract-checks.git
    GIT_TAG        <this library's release version>
  )
  FetchContent_MakeAvailable(adobe-contract-checks)

  # Set adobe-contract-checks configuration default based on build
  # type.
  if(CMAKE_BUILD_TYPE EQUALS "Debug")
    set(default_ADOBE_CONTRACT_VIOLATION "verbose")
  else()
    set(default_ADOBE_CONTRACT_VIOLATION "lightweight")
  endif()
  # declare the option so user can configure on CMake command-line or
  # in CMakeCache.txt.
  option(ADOBE_CONTRACT_VIOLATION
    "Behavior when a contract violation is detected"
    "${default_ADOBE_CONTRACT_VIOLATION}")
endif()
find_package(adobe-contract-checks)

# Configure usage of this library by all targets the same way.
# (repeated in each CMakeLists.txt that adds C++ targets).
if(DEFINED ADOBE_CONTRACT_VIOLATION)
  add_compile_definitions(
    "ADOBE_CONTRACT_VIOLATION=${ADOBE_CONTRACT_VIOLATION}")
endif()

# --- your project's targets -----

add_library(my-library my-library.cpp)
target_link_libraries(my-library PRIVATE adobe-contract-checks)

add_executable(my-executable my-executable.cpp)
target_link_libraries(my-executable PRIVATE adobe-contract-checks)
```

## Recommendations

- Start by checking whatever you can, and worry about performance
  later. Checks are often critical for safety. [Configuration
  options](#configuration) can be used to mitigate or eliminate costs
  later if necessary. That said, **do not add checks that change
  algorithmic complexity**. Turning an O(1) operation into an O(n)
  operation is not acceptable.

- If you have to prioritize, precondition checks are the most
  important; they are your last line of defense against undefined
  behavior.

  Class invariant checks can often give you more bang for your buck,
  though, because they can be used to eliminate the need for
  precondition checks and verbose documentation across many
  functions. For example, the second function below benefits by
  accepting a `date` type whose invariant ensures its validity.

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

- The conditions in your checks should not have side-effects that
  change program behavior; readers expect to be able to skip over
  these checks when reasoning about code.

- Group all precondition checks immediately after a function's
  opening brace, and don't allow any code to sneak in before them.

- Give your `struct` or `class` a `bool is_valid() const` method that
  returns `true` if and only if invariants are intact, so that
  invariant condition checking can be centralized. Invoke
  `ADOBE_INVARIANT(is_valid())` from each public mutating friend or
  member function or constructor just before each `return`, or before
  `*this` becomes visible to any other component such as a callback
  parameter... except in the case where an error is reported
  ([rationale](#how-reported-errors-fit-in)).

  Base classes should define a `protected` `virtual` `is_valid()`
  member. Derived class overrides should begin by checking
  `is_valid()`(s) of their base classes.

  ```c++
      bool is_valid() const override {
        return base::is_valid() && some_derived_class_invariant();
      }
  ```

- If your program needs to take <a name="emergency-shutdown">emergency
  shutdown</a> measures before termination, put those in a [terminate
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
  of undefined behavior should the check be skipped. In that case,
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
unit tests, and many good frameworks for unit testing exist. Adding a
postcondition check to this library would just create confusion about
where postcondition checks belong and about the purpose of unit
testing. Also, postcondition checks for most mutating functions need to
make an initial copy of everything being mutated, which can be
prohibitively expensive even for debug builds.

### Why This Library Does Not Throw Exceptions

In the original [Eiffel](<https://en.wikipedia.org/wiki/Eiffel_(programming_language)>)
programming language implementation of [Design by
Contract](https://en.wikipedia.org/wiki/Design_by_contract), a contract
violation would cause an
exception to be thrown. On the surface, that might seem at first like
a good response to bug detection, but there are several problems:

- It's important that some functions, such as those used in
  destructors, `catch` blocks, or `noexcept` functions, never throw
  exceptions. Contract checking is useful everywhere.
- Exceptions cause stack unwinding, which destroys information that
  otherwise could be vital to finding the cause of a bug.
- Exceptions are for conditions where the program can, after taking
  some recovery action, return to normal execution. If there's no path
  back to normal execution, we might as well terminate the program
  (possibly after taking emergency measures). But when a bug is
  discovered, the potential damage to program state is arbitrary, and
  there is no known recovery action. Also see the section on
  [defensive programming](#about-defensive-programming) below.

If your function really needs to throw an exception, that should be a
documented part of its contract, so that response can be tested for
and callers can respond appropriately. See [How Reported Errors Fit
In](#how-reported-errors-fit-in) for more information.

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

The usual procedures for development with CMake apply. One typical
set of commands might be:

```sh
cmake -DBUILD_TESTING=1 -Wno-dev -S . -B ../build -GNinja
cmake --build ../build
ctest --output-on-failure --test-dir ../build
```

**VSCode CMake Plugin Note**: Test Explorer integration has been disabled in the `.vscode/settings.json` file, because it [breaks test detection](https://github.com/microsoft/vscode-cmake-tools/issues/3358). If you see `_NOT_BUILT` in the CTest output, test explorer integration was somehow enabled at configure time.
