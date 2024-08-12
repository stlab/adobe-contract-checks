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

include(GoogleTest)

# Adds a test called verify_<base_name>, built from <base_name>.cpp
# with test executable target named <base_name>, that passes iff it
# calls abort() and outputs a string matching the regular expression
# <expected_output_re>.
function(adobe_contract_checking_add_test base_name)
  add_executable("${base_name}" "${base_name}.cpp" abort_detection.cpp)
  adobe_contract_checking_apply_standard_options("${base_name}")
  target_link_libraries(
    "${base_name}"
    PRIVATE
    adobe-contract-checking
    GTest::gtest_main)
  gtest_discover_tests("${base_name}")

  if(WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
      TARGET "${base_name}"
      PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:"${base_name}"> $<TARGET_FILE_DIR:"${base_name}">
      COMMAND_EXPAND_LISTS)
  endif()
endfunction()

adobe_contract_checking_add_test(precondition_tests)
adobe_contract_checking_add_test(throwing_tests)
