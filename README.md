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
  code.  The most common and useful kind of invariants are **class
  invariants**, which hold at any point where an instance can be
  inspected from outside the class.

A function's specification must at least describe its preconditions
and postconditions, and the specification of a class must describe its
publicly-visible invariants.  Additionally describing these conditions
in code and checking them at runtime can be  a powerful way to catch
bugs early and prevent their damaging effects.

## Basic C++ Usage

This is a header-only library.  To use it from C++, simply put the
`include` directory of this repository in your `#include` path, and
`#include <adobe/contract_checks.hpp>`.

```c++
#include <adobe/contract_checks.hpp>
```

Every executable needs a single contract violation handler that
determines the program's behavior when a violation is detected. A
good starting point is provided by a macro that you can expand in a
source file such as the one containing your `main` function.

```c++
ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER() // no semicolon
```

There are three primary macros used to check contracts, each with two
forms: `ADOBE_PRECONDITION`, `ADOBE_POSTCONDITION`, and
`ADOBE_INVARIANT`. Each has one required argument and one optional
argument:

- `condition`: an expression convertible to `bool`; if `false`, the
  violation handler is invoked.
- `message`: an expression convertible to `const char*` pointing to a
  [null-terminated](https://en.cppreference.com/w/cpp/string/byte)
  message that is additionally passed to the violation handler. The
  default value is the empty string, `""`.

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
    ADOBE_POSTCONDITION(end() == old_end + 1);
    check_invariant();
  }

  // more methods...
};
```

## CMake Usage

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

### Defining a contract violation handler

If you don't use `ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()` to
inject a definition, you'll need to define this function
with external linkage:

```c++
     [[noreturn]] void ::adobe::contract_violated(
       const char *const condition,
       int kind,
       const char *const file,
       std::uint32_t const line,
       const char *const message) noexcept;
```

The parameters are as follows:

- `condition`: a [null-terminated byte
  string](https://en.cppreference.com/w/cpp/string/byte) string
  containing the text of the checked condition, or `""` if
  [`ADOBE_NO_CONTRACT_CONDITION_STRINGS`](#symbols-that-minimize-generated-code-and-data)
  is `#defined`.
- `kind`: 1 for a precondition, 2 for a postcondition, or 3 for an invariant.
- `file`: a [null-terminated byte
  string](https://en.cppreference.com/w/cpp/string/byte) string
  containing the name of the source file with the failing check as it
  was passed to the compiler, or `""` if
  [`ADOBE_NO_CONTRACT_FILENAME_STRINGS`](#symbols-that-minimize-generated-code-and-datat)
  is `#defined`.
- `line`: the line number on which the failing check was written.
- `message`: the second argument to the failing check macro, or "" if
  none was passed or if
  [`ADOBE_NO_CONTRACT_MESSAGE_STRINGS`](#symbols-that-minimize-generated-code-and-data)
  is `#defined`.

If, against our advice (see [recommendation 1](#recommendations)) you
choose to throw an exception in response to a contract violation, you
will omit `noexcept`, and `#define`
[`ADOBE_CONTRACT_VIOLATED_THROWS`](#contract-handler-definition).

In release builds you may wish to use a more minimal inline violation
handler, in which case you should define it in your
[`ADOBE_CONTRACT_CHECKS_CONFIGURATION`](#configuration) file.  Note
that defining a more complex handler inline usually will increase
binary sizes and hurt performance.

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

3. If you have to prioritize, precondition checks are the most
   important; they are your last line of defense against undefined
   behavior.  Postcondition checks overlap somewhat with what will be
   checked by unit tests.  They still provide value because unit tests
   don't cover all possible inputs and the checks may fire outside of
   testing.

   Class invariant checks can give you the most bang for your buck
   because they can be used to eliminate the need for precondition
   checks and verbose documentation across many functions.

   ```c++
   // Returns the day of the week corresponding to the date described
   // by "<year>-<month>-<day>" (interpreted in ISO standard date
   // format)
   day_of_the_week day(int year, int month, int day) {
     ADOBE_PRECONDITION(is_valid_date(year, month, day));
     // implementation starts here.
   }

   // Returns the day of the week corresponding to `d`.
   day_of_the_week day(date d) {
     // implementation starts here.
   }
   ```

   The second function above benefits by accepting a `date` type whose
   invariant ensures its validity.

4. The conditions in your checks should not have side-effects that
   change program behavior, because checks are sometimes turned off by
   configuration.

2. Group all precondition checks immediately after a function's
   opening brace, and don't allow any code to sneak in before them.

3. Group all postcondition checks just before your function returns.
   (That may mean temporarily storing a return value in a local
   variable so it can be tested.)

4. Give your `struct` or `class` a `void check_invariant() const`
   method containing `ADOBE_INVARIANT` invocations.  Invoke it from each 
   public mutating friend or member function, just before returning, and just before passing 
   access to `*this` to any component outside the class.

6. If a function throws exceptions or can otherwise report an error **to
   its caller**, don't call that a precondition violation.  Instead,
   make that behavior part of the function's specification: document
   the conditions, the resulting behavior, and test it to make sure
   that it works.

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
       const char *const message) noexcept
     {
       // whatever you want here.
       std::terminate();
     }
     ```

   That way, other reasons for unexpected termination, such as uncaught
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

5. Don't disable critical checks in shipping code unless a measurable
   unacceptable performance cost is found.  In that case, disable the
   expensive checks selectively, e.g.

   ```
   #ifndef NDEBUG // too expensive for release
   ADOBE_PRECONDITION(some_expensive_call());
   #endif
   ```

6. Use `ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()`, or a custom
   handler derived from it, in your debug builds.  For release builds,
   use `ADOBE_DEFAULT_CONTRACT_VIOLATION_HANDLER()` if you can afford
   to.  To get a sense of how much smaller and efficient code can
   possibly get without turning off checks, define an inline minimal
   handler and [disable condition, message, and filename
   strings](#symbols-that-minimize-generated-code-and-data) (see the
   example project for how to do this).  Then you can make an informed
   decision about what kind of handler to use and how much information
   to generate.

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

The usual procedures for development with cmake apply.  One typical
set of commands might be:

```sh
cmake -Wno-dev -S . -B ../build -GNinja        # configure
cmake --build ../build                         # build/rebuild after changes
ctest --output-on-failure --test-dir ../build  # test
```

## Reference

- `ADOBE_MINIMAL_TRAP()`: Invoke this macro to inject minimal code
  that stops your program; it's usually just one machine instruction
  and more efficient than calling `terminate()`.  You'd typically use
  this macro from a minimal contract violation handler (see the
  example project).

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
your `CMakeLists.txt` or on the command line will cause all clients of this
library in your build to use that configuration file.

This library can only have one configuration in an executable, so the
privilege of configuring it belongs to the executable.  In CMake,

```cmake
if(PROJECT_IS_TOP_LEVEL)
  set(ADOBE_CONTRACT_CHECKS_CONFIGURATION "<myproject/adobe_contract_checks_config.hpp>")
endif()
```

#### Preprocessor configuration symbols

##### Contract violation handler definition

- `ADOBE_CONTRACT_VIOLATED_THROWS`: define this symbol if your
  contract violation handler, against our advice (see [recommendation
  1](#recommendations)), can throw exceptions.

##### Symbols that minimize generated code and data

- `ADOBE_SKIP_NONCRITICAL_PRECONDITION_CHECKS`: define this symbol to
  make uses of `ADOBE_NONCRITICAL_PRECONDITION` generate no code.  Be
  sure you use `ADOBE_NONCRITICAL_PRECONDITION` according to its
  documentation.

- `ADOBE_SKIP_ALL_CONTRACT_CHECKS`: define this symbol to make all
  contract checking macros generate no code.  Not recommended for
  general use, but can be useful for measuring the overall performance
  impact of checking in a program.

The following symbols change the behavior of the contract checking
macros so that they generate lighter-weight (and less informative)
calls to the violation handler.  For example, if you put expensive
string construction expressions in the second arguments to these
macros, but define `ADOBE_NO_CONTRACT_MESSAGE_STRINGS`, those
expressions will never be evaluated.

- `ADOBE_NO_CONTRACT_CONDITION_STRINGS`: define this symbol to
  suppress the generation of strings describing failed check
  conditions.  The empty string will be used instead.

- `ADOBE_NO_CONTRACT_MESSAGE_STRINGS`: define this symbol to suppress
  the expansion of `message` arguments to checking macros.

- `ADOBE_NO_CONTRACT_FILENAME_STRINGS`: define this symbol to suppress
  the generation of strings describing the file in which failed checks
  occurred.  `"<unknown file>"` will be used instead.

------------------

[![ci](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/ci.yml)
[![CodeQL](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/stlab/adobe-contract-checks/actions/workflows/codeql-analysis.yml)


## More Details

 * [Dependency Setup](README_dependencies.md)
 * [Building Details](README_building.md)
 * [Troubleshooting](README_troubleshooting.md)
 * [Docker](README_docker.md)
