include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(adobe_contract_checking_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(adobe_contract_checking_setup_options)
  option(adobe_contract_checking_ENABLE_HARDENING "Enable hardening" ON)
  option(adobe_contract_checking_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    adobe_contract_checking_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    adobe_contract_checking_ENABLE_HARDENING
    OFF)

  adobe_contract_checking_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR adobe_contract_checking_PACKAGING_MAINTAINER_MODE)
    option(adobe_contract_checking_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(adobe_contract_checking_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(adobe_contract_checking_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(adobe_contract_checking_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(adobe_contract_checking_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(adobe_contract_checking_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(adobe_contract_checking_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(adobe_contract_checking_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(adobe_contract_checking_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(adobe_contract_checking_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(adobe_contract_checking_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(adobe_contract_checking_ENABLE_PCH "Enable precompiled headers" OFF)
    option(adobe_contract_checking_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(adobe_contract_checking_ENABLE_IPO "Enable IPO/LTO" ON)
    option(adobe_contract_checking_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(adobe_contract_checking_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(adobe_contract_checking_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(adobe_contract_checking_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(adobe_contract_checking_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(adobe_contract_checking_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(adobe_contract_checking_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(adobe_contract_checking_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(adobe_contract_checking_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(adobe_contract_checking_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(adobe_contract_checking_ENABLE_PCH "Enable precompiled headers" OFF)
    option(adobe_contract_checking_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      adobe_contract_checking_ENABLE_IPO
      adobe_contract_checking_WARNINGS_AS_ERRORS
      adobe_contract_checking_ENABLE_USER_LINKER
      adobe_contract_checking_ENABLE_SANITIZER_ADDRESS
      adobe_contract_checking_ENABLE_SANITIZER_LEAK
      adobe_contract_checking_ENABLE_SANITIZER_UNDEFINED
      adobe_contract_checking_ENABLE_SANITIZER_THREAD
      adobe_contract_checking_ENABLE_SANITIZER_MEMORY
      adobe_contract_checking_ENABLE_UNITY_BUILD
      adobe_contract_checking_ENABLE_CLANG_TIDY
      adobe_contract_checking_ENABLE_CPPCHECK
      adobe_contract_checking_ENABLE_COVERAGE
      adobe_contract_checking_ENABLE_PCH
      adobe_contract_checking_ENABLE_CACHE)
  endif()

  adobe_contract_checking_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (adobe_contract_checking_ENABLE_SANITIZER_ADDRESS OR adobe_contract_checking_ENABLE_SANITIZER_THREAD OR adobe_contract_checking_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(adobe_contract_checking_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(adobe_contract_checking_global_options)
  if(adobe_contract_checking_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    adobe_contract_checking_enable_ipo()
  endif()

  adobe_contract_checking_supports_sanitizers()

  if(adobe_contract_checking_ENABLE_HARDENING AND adobe_contract_checking_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR adobe_contract_checking_ENABLE_SANITIZER_UNDEFINED
       OR adobe_contract_checking_ENABLE_SANITIZER_ADDRESS
       OR adobe_contract_checking_ENABLE_SANITIZER_THREAD
       OR adobe_contract_checking_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${adobe_contract_checking_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${adobe_contract_checking_ENABLE_SANITIZER_UNDEFINED}")
    adobe_contract_checking_enable_hardening(adobe_contract_checking_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(adobe_contract_checking_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(adobe_contract_checking_warnings INTERFACE)
  add_library(adobe_contract_checking_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  adobe_contract_checking_set_project_warnings(
    adobe_contract_checking_warnings
    ${adobe_contract_checking_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(adobe_contract_checking_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    adobe_contract_checking_configure_linker(adobe_contract_checking_options)
  endif()

  include(cmake/Sanitizers.cmake)
  adobe_contract_checking_enable_sanitizers(
    adobe_contract_checking_options
    ${adobe_contract_checking_ENABLE_SANITIZER_ADDRESS}
    ${adobe_contract_checking_ENABLE_SANITIZER_LEAK}
    ${adobe_contract_checking_ENABLE_SANITIZER_UNDEFINED}
    ${adobe_contract_checking_ENABLE_SANITIZER_THREAD}
    ${adobe_contract_checking_ENABLE_SANITIZER_MEMORY})

  set_target_properties(adobe_contract_checking_options PROPERTIES UNITY_BUILD ${adobe_contract_checking_ENABLE_UNITY_BUILD})

  if(adobe_contract_checking_ENABLE_PCH)
    target_precompile_headers(
      adobe_contract_checking_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(adobe_contract_checking_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    adobe_contract_checking_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(adobe_contract_checking_ENABLE_CLANG_TIDY)
    adobe_contract_checking_enable_clang_tidy(adobe_contract_checking_options ${adobe_contract_checking_WARNINGS_AS_ERRORS})
  endif()

  if(adobe_contract_checking_ENABLE_CPPCHECK)
    adobe_contract_checking_enable_cppcheck(${adobe_contract_checking_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(adobe_contract_checking_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    adobe_contract_checking_enable_coverage(adobe_contract_checking_options)
  endif()

  if(adobe_contract_checking_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(adobe_contract_checking_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(adobe_contract_checking_ENABLE_HARDENING AND NOT adobe_contract_checking_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR adobe_contract_checking_ENABLE_SANITIZER_UNDEFINED
       OR adobe_contract_checking_ENABLE_SANITIZER_ADDRESS
       OR adobe_contract_checking_ENABLE_SANITIZER_THREAD
       OR adobe_contract_checking_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    adobe_contract_checking_enable_hardening(adobe_contract_checking_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()

# Applies the standard project options to <target>.
function(adobe_contract_checking_apply_standard_options target)
  target_link_libraries(
    ${target}
    PRIVATE adobe_contract_checking::adobe_contract_checking_warnings
    adobe_contract_checking::adobe_contract_checking_options)
endfunction()
