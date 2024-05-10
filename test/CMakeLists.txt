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

add_executable(aborts aborts.cpp abort_detection.cpp)
add_test(NAME aborts_test COMMAND $<TARGET_FILE:aborts>)
set_tests_properties(aborts_test PROPERTIES
  PASS_REGULAR_EXPRESSION "##ABORTED##"
)

add_executable(precondition_failure precondition_failure.cpp abort_detection.cpp)
target_link_libraries(precondition_failure adobe-contract-checking)
add_test(NAME precondition_failure_test COMMAND $<TARGET_FILE:precondition_failure>)
set_tests_properties(precondition_failure_test PROPERTIES
  PASS_REGULAR_EXPRESSION "##ABORTED##"
)


add_executable(tests tests.cpp)
target_link_libraries(
  tests
  PRIVATE myproject::myproject_warnings
          myproject::myproject_options
          myproject::sample_library
          Catch2::Catch2WithMain)

if(WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(
    TARGET tests
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
    COMMAND_EXPAND_LISTS)
endif()

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

