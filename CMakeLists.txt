cmake_minimum_required(VERSION 3.21)

# This template attempts to be "fetch_content"-able
# so that it works well with tools like CPM or other
# manual dependency management

# Only set the cxx_standard if it is not set by someone else
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the project name and language
project(
  adobe-contract-checking
  VERSION 0.0.1
  DESCRIPTION "C++ contract checking from Adobe"
  HOMEPAGE_URL "https://github.com/stlab/adobe-contract-checks"
  LANGUAGES CXX)

include(cmake/PreventInSourceBuilds.cmake)
include(ProjectOptions.cmake)


adobe_contract_checking_setup_options()

adobe_contract_checking_global_options()
include(Dependencies.cmake)

adobe_contract_checking_local_options()

# don't know if this should be set globally from here or not...
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(GIT_SHA
    "Unknown"
    CACHE STRING "SHA this build was generated from")
string(
  SUBSTRING "${GIT_SHA}"
            0
            8
            GIT_SHORT_SHA)

target_compile_features(adobe_contract_checking_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

add_library(adobe_contract_checking::adobe_contract_checking_options ALIAS adobe_contract_checking_options)
add_library(adobe_contract_checking::adobe_contract_checking_warnings ALIAS adobe_contract_checking_warnings)

#add_library(adobe_contract_checking::adobe_contract_checking_options INTERFACE IMPORTED)
#add_library(adobe_contract_checking::adobe_contract_checking_warnings INTERFACE IMPORTED)

# configure files based on CMake configuration options
add_subdirectory(configured_files)

# Adding the src:

# STUFF WE WANT TO KEEP
#
# adobe-contract-checking has no compiled components. As such, we
# declare it as an `INTERFACE` library, which denotes a collection of
# target propeties to be applied transitively to linking targets. In
# our case, this amounts to an include directory, compile flags,
# linking flags, and links to system libraries.
#
add_library( adobe-contract-checking INTERFACE )
add_library( adobe-contract-checking::adobe-contract-checking ALIAS adobe-contract-checking )

#
# The include directory for adobe-contract-checking can be expected to
# vary between build and installaion. Here we use a CMake generator
# expression to dispatch on the configuration under which this
# library is being consumed.
#
target_include_directories( adobe-contract-checking INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include> )

target_sources( adobe-contract-checking INTERFACE
  FILE_SET adobe_contract_checking
  TYPE HEADERS
  BASE_DIRS include/adobe
  FILES contract-checking.hpp
)

# Don't even look at tests if we're not top level
if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

if(BUILD_TESTING)
  include(CTest)
  add_subdirectory(test)
endif()


# If MSVC is being used, and ASAN is enabled, we need to set the debugger environment
# so that it behaves well with MSVC's debugger, and we can run the target from visual studio
if(MSVC)
  get_all_installable_targets(all_targets)
  message("all_targets=${all_targets}")
  set_target_properties(${all_targets} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
endif()

# set the startup project for the "play" button in MSVC
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT intro)

if(CMAKE_SKIP_INSTALL_RULES)
  return()
endif()

include(cmake/PackageProject.cmake)

# Add other targets that you want installed here, by default we just package the one executable
# we know we want to ship
adobe_contract_checking_package_project(
  TARGETS
  adobe_contract_checking_options
  adobe_contract_checking_warnings
  # FIXME: this does not work! CK
  # PRIVATE_DEPENDENCIES_CONFIGURED project_options project_warnings
)

# Experience shows that explicit package naming can help make it easier to sort
# out potential ABI related issues before they start, while helping you
# track a build to a specific GIT SHA
set(CPACK_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${GIT_SHORT_SHA}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
)

include(CPack)
