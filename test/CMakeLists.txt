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

# include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# Adds a test called verify_<base_name>, built from <base_name>.cpp
# with test executable target named <base_name>, that passes iff it
# calls abort() and outputs a string matching the regular expression
# <expected_output_re>.
function(adobe_contract_checking_add_abort_test base_name expected_output_re)
  add_executable(${base_name} ${base_name}.cpp abort_detection.cpp)
  adobe_contract_checking_apply_standard_options(${base_name})
  target_link_libraries(${base_name} PRIVATE adobe-contract-checking)
  # we pass through the cmake command because typical break behaviors
  # include raising SIGILL, which will cause CMake to report the test
  # as a failure despite the WILL_FAIL setting.
  add_test(NAME verify_${base_name} COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:${base_name}>)
  set_tests_properties(verify_${base_name} PROPERTIES
    WILL_FAIL YES
    # PASS_REGULAR_EXPRESSION "${expected_output_re}.*##ABORTED##"
  )
endfunction()

# Adds a test called verify_<base_name>, built from <base_name>.cpp
# with test executable target named <base_name>, that passes iff it
# exits with an error and does not output a string matching regular
# expression <unexpected_output_re>.
function(adobe_contract_checking_add_minimal_failure_test base_name unexpected_output_re)
  add_executable(${base_name} ${base_name}.cpp)
  adobe_contract_checking_apply_standard_options(${base_name})
  target_link_libraries(${base_name} PRIVATE adobe-contract-checking)
  # we pass through the cmake command because typical break behaviors
  # include raising SIGILL, which will cause CMake to report the test
  # as a failure despite the WILL_FAIL setting.
  add_test(NAME verify_${base_name} COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:${base_name}>)
  set_tests_properties(verify_${base_name} PROPERTIES
    WILL_FAIL YES
    # PASS_REGULAR_EXPRESSION "${unexpected_output_re}"
  )
endfunction()

adobe_contract_checking_add_abort_test(abort_is_detected "")

adobe_contract_checking_add_abort_test(
  precondition_failure_aborts "precondition")

adobe_contract_checking_add_abort_test(
  precondition_failure_output
  "precondition_failure_output\.cpp:9999: Precondition violated [(]false[)]")

adobe_contract_checking_add_abort_test(
  precondition_failure_with_message_output
  "precondition_failure_with_message_output\.cpp:9999: Precondition violated \(false\)\. expected message")

add_executable(throwing_tests throwing_tests.cpp)
adobe_contract_checking_apply_standard_options(throwing_tests)
target_link_libraries(
  throwing_tests
  PRIVATE
  adobe-contract-checking)

add_test(NAME verify_throwing_tests COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:throwing_tests>)
set_tests_properties(verify_throwing_tests PROPERTIES
  WILL_FAIL NO
  PASS_REGULAR_EXPRESSION "passed"

adobe_contract_checking_add_minimal_failure_test(
  precondition_failure_minimal "")
)
