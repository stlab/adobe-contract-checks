#ifndef ADOBE_INTERNAL_TEST_MACRO_UTILITIES_HPP

/// turns `x` into a string literal.
#define ADOBE_INTERNAL_STRINGIZE(x) \
  ADOBE_INTERNAL_STRINGIZE2(x)// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADOBE_INTERNAL_STRINGIZE2(x) #x// NOLINT(cppcoreguidelines-macro-usage)


/// a string literal for __LINE__.
#define ADOBE_INTERNAL_LINE_STRING() \
  ADOBE_INTERNAL_STRINGIZE(__LINE__)// NOLINT(cppcoreguidelines-macro-usage)

#endif
