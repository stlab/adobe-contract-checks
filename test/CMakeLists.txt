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


if(EMSCRIPTEN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fexceptions")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
endif()

# ---- Dependencies ----

include(GoogleTest)

# Adds a test called verify_<base_name> with test executable target
# named <base_name>, that passes iff it exits as expected and outputs
# a string matching the regular expression <expected_output_re>. The
# test source file list is given as additional arguments, or defaults
# to "${base_name}.cpp".
function(adobe_contract_checking_add_test base_name)
  if(ARGN)
    set(sources ${ARGN})
  else()
    set(sources "${base_name}.cpp")
  endif()

  add_executable("${base_name}"
    ${sources}
    win32_abort_detection.cpp no_external_linkage_assurance.cpp)
  adobe_contract_checking_apply_standard_options("${base_name}")
  target_link_libraries(
    "${base_name}"
    PRIVATE
    adobe-contract-checking
    GTest::gtest_main gmock)
  gtest_discover_tests("${base_name}")
  if(EMSCRIPTEN)
    handle_emscripten_death_tests("${base_name}")
  endif()
  if(WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
      TARGET "${base_name}"
      PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:"${base_name}"> $<TARGET_FILE_DIR:"${base_name}">
      COMMAND_EXPAND_LISTS)
  endif()
endfunction()

# Turns all tests discovered in <test_executable> by
# gtest_discover_tests that are named with the "DeathTest" convention
# into CMake WILL_FAIL tests.
#
# It is expected that under emscripten, these tests are actually
# compiled as non-death tests; i.e. they simply execute the code that
# is expected to die.  There is no support for checking that the exit
# code or output meets expectations.
function(handle_emscripten_death_tests test_executable)
  set(ctest_include_file "${CMAKE_CURRENT_BINARY_DIR}/${test_executable}_death.cmake")
  file(WRITE "${ctest_include_file}"
    "set(death_tests \"\${${test_executable}_TESTS}}\")\n"
    "list(FILTER death_tests INCLUDE REGEX \"DeathTest\")\n"
    "set_tests_properties(\${death_tests} PROPERTIES WILL_FAIL YES)\n"
  )
  set_property(DIRECTORY
    APPEND PROPERTY TEST_INCLUDE_FILES "${ctest_include_file}"
  )
endfunction()

adobe_contract_checking_add_test(verbose_configuration_tests)
target_compile_definitions(verbose_configuration_tests PRIVATE
  "ADOBE_CONTRACT_VIOLATION=verbose")

adobe_contract_checking_add_test(lightweight_configuration_tests)
target_compile_definitions(lightweight_configuration_tests PRIVATE
  "ADOBE_CONTRACT_VIOLATION=lightweight")

adobe_contract_checking_add_test(unsafe_configuration_tests)
target_compile_definitions(unsafe_configuration_tests PRIVATE
  "ADOBE_CONTRACT_VIOLATION=unsafe")
