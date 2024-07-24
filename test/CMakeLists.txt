cmake_minimum_required(VERSION 3.15...3.23)

project(CmakeConfigPackageTests LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----

if(PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)
  enable_testing()

  find_package(myproject CONFIG REQUIRED) # for intro, project_options, ...

  if(NOT TARGET myproject_options)
    message(FATAL_ERROR "Required config package not found!")
    return() # be strictly paranoid for Template Janitor github action! CK
  endif()
endif()

# ---- Dependencies ----

include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# Adds a test called <base_name>_test, built from <base_name>.cpp with
# test executable target named <base_name>, that fails unless it calls
# abort() with a string matching the regular expression
# <expected_output_re> embedded in the test's output.
function(add_abort_test base_name expected_output_re)
  add_executable(${base_name} ${base_name}.cpp abort_detection.cpp)
  myproject_apply_standard_options(${base_name})
  target_link_libraries(${base_name} PRIVATE adobe-contract-checking)
  add_test(NAME ${base_name}_test COMMAND $<TARGET_FILE:${base_name}>)
  set_tests_properties(${base_name}_test PROPERTIES
    PASS_REGULAR_EXPRESSION "${expected_output_re}.*##ABORTED##"
  )
endfunction()

add_abort_test(aborts "")
add_abort_test(precondition_failure "precondition")

if(NO)
  add_executable(tests tests.cpp)
  target_link_libraries(
    tests
    PRIVATE myproject::myproject_warnings
    myproject::myproject_options
    myproject::sample_library
    Catch2::Catch2WithMain)
endif()

if(WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(
    TARGET tests
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
    COMMAND_EXPAND_LISTS)
endif()

if(NO)
  # automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
  # to whatever you want, or use different for different binaries
  catch_discover_tests(
    tests
    TEST_PREFIX
    "unittests."
    REPORTER
    XML
    OUTPUT_DIR
    .
    OUTPUT_PREFIX
    "unittests."
    OUTPUT_SUFFIX
    .xml)
endif()
