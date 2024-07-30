cmake_minimum_required(VERSION 3.15...3.23)

project(CmakeConfigPackageTests LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----

if(PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)
  enable_testing()

  find_package(adobe_contract_checking CONFIG REQUIRED) # for intro, project_options, ...

  if(NOT TARGET adobe_contract_checking_options)
    message(FATAL_ERROR "Required config package not found!")
    return() # be strictly paranoid for Template Janitor github action! CK
  endif()
endif()

# ---- Dependencies ----

include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# Adds a test called verify_<base_name>, built from <base_name>.cpp with
# test executable target named <base_name>, that fails unless it calls
# abort() with a string matching the regular expression
# <expected_output_re> embedded in the test's output.
function(adobe_contract_checking_add_abort_test base_name expected_output_re)
  add_executable(${base_name} ${base_name}.cpp abort_detection.cpp)
  adobe_contract_checking_apply_standard_options(${base_name})
  target_link_libraries(${base_name} PRIVATE adobe-contract-checking)
  add_test(NAME verify_${base_name} COMMAND $<TARGET_FILE:${base_name}>)
  set_tests_properties(verify_${base_name} PROPERTIES
    PASS_REGULAR_EXPRESSION "${expected_output_re}.*##ABORTED##"
  )
endfunction()

adobe_contract_checking_add_abort_test(abort_is_detected "")

adobe_contract_checking_add_abort_test(
  precondition_failure_aborts "precondition")

adobe_contract_checking_add_abort_test(
  precondition_failure_prints_file_and_line
  "precondition_failure_prints_file_and_line.cpp:9999: Precondition violated [(]1 < 0[)]")

adobe_contract_checking_add_abort_test(
  precondition_failure_with_message_prints_file_line_and_message
  "precondition_failure_with_message_prints_file_line_and_message.cpp:9999: Precondition violated [(]1 < 0[)]. expected message")

add_executable(throwing_tests throwing_tests.cpp)
adobe_contract_checking_apply_standard_options(throwing_tests)
target_link_libraries(
  throwing_tests
  PRIVATE
  # adobe_contract_checking::sample_library
  adobe-contract-checking
  Catch2::Catch2WithMain)

if(WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(
    TARGET throwing_tests
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:throwing_tests> $<TARGET_FILE_DIR:throwing_tests>
    COMMAND_EXPAND_LISTS)
endif()

# automatically discover tests that are defined in catch based test files you can modify the
# unittests. Set TEST_PREFIX to whatever you want, or use different for different binaries
catch_discover_tests(
  throwing_tests
  # setting TEST_PREFIX seems to be incompatible with VSCode integrations.

  # TEST_PREFIX
  # "unittests."
  REPORTER
  XML
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "unittests."
  OUTPUT_SUFFIX
  .xml)
