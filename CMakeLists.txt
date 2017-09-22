# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# We require at least CMake V3.3.2
cmake_minimum_required(VERSION 3.3.2 FATAL_ERROR)

# explicitly set certain policies
cmake_policy(VERSION 3.3.2)

################################################################################
# Build type (needs to be handled before project command below)
################################################################################
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Configuration type (one of Debug, RelWithDebInfo, Release, MinSizeRel)")
endif()

################################################################################
# project metadata
################################################################################
project(phylanx CXX C)

set(PHYLANX_MAJOR_VERSION 0)
set(PHYLANX_MINOR_VERSION 0)
set(PHYLANX_PATCH_LEVEL   1)

set(PHYLANX_VERSION "${PHYLANX_MAJOR_VERSION}.${PHYLANX_MINOR_VERSION}.${PHYLANX_PATCH_LEVEL}")
set(PHYLANX_LIBRARY_VERSION "${PHYLANX_VERSION}")
set(PHYLANX_SOVERSION ${PHYLANX_MAJOR_VERSION})
set(PHYLANX_PACKAGE_NAME phylanx)

if(MSVC)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

################################################################################
# CMake configuration
################################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

################################################################################
# Set basic search paths for Phylanx
################################################################################
include_directories("${PROJECT_SOURCE_DIR}")
link_directories(${CMAKE_BINARY_DIR}/lib)

# Debug library postfix
set(CMAKE_DEBUG_POSTFIX "d")
set(PHYLANX_DEBUG_POSTFIX "d")

include(CheckCXXCompilerFlag)
include(CMakeDependentOption)

# include additional macro definitions
include(Phylanx_Utils)

phylanx_force_out_of_tree_build(
  "This project requires an out-of-source-tree build. See README.rst. "
  "Clean your CMake cache and CMakeFiles if this message persists.")

if(NOT PHYLANX_CMAKE_LOGLEVEL)
  set(PHYLANX_CMAKE_LOGLEVEL "WARN")
endif()

# print initial diagnostics
phylanx_info("CMake version: " ${CMAKE_VERSION})
phylanx_info("Phylanx version: " ${PHYLANX_VERSION})

################################################################################
# Locate dependencies
if(NOT HPX_DIR AND HPX_ROOT)
  set(HPX_DIR ${HPX_ROOT}/lib/cmake/HPX)
endif()

if(EXISTS "${HPX_DIR}")
  find_package(HPX REQUIRED NO_CMAKE_PACKAGE_REGISTRY)

  if(NOT HPX_FOUND)
    phylanx_error("HPX could not be found, please set HPX_DIR to help locating it.")
  endif()

  # make sure that configured build type for Phylanx matches the one used for HPX
  if(NOT (${HPX_BUILD_TYPE} STREQUAL ${CMAKE_BUILD_TYPE}))
    list(FIND ${CMAKE_BUILD_TYPE} ${HPX_BUILD_TYPE} __pos)
    if(${__pos} EQUAL -1)
      phylanx_warn(
        "The configured CMAKE_BUILD_TYPE (${CMAKE_BUILD_TYPE}) is "
        "different from the build type used for the found HPX"
        "(HPX_BUILD_TYPE: ${HPX_BUILD_TYPE})")
    endif()
  endif()

  include_directories(${HPX_INCLUDE_DIRS})
  link_directories(${HPX_LIBRARY_DIR})

  phylanx_info("HPX version: " ${HPX_VERSION_STRING})

else()
  phylanx_error("HPX_DIR has not been specified, please set it to help locating HPX")
endif()

find_package(Eigen3 REQUIRED NO_CMAKE_PACKAGE_REGISTRY)
if(NOT Eigen3_FOUND AND NOT EIGEN3_FOUND)
  phylanx_error("Eigen3 could not be found, please set Eigen3_DIR to help locating it.")
endif()
if(NOT ${EIGEN3_VERSION_STRING})
  set(EIGEN3_VERSION_STRING ${Eigen3_VERSION})
endif()
if(Eigen3_INCLUDE_DIRS)
  include_directories(${Eigen3_INCLUDE_DIRS})
endif()
if(EIGEN3_INCLUDE_DIRS)
  include_directories(${EIGEN3_INCLUDE_DIRS})
endif()
phylanx_info("Eigen3 was found, version: " ${EIGEN3_VERSION_STRING})

find_package(pybind11 REQUIRED NO_CMAKE_PACKAGE_REGISTRY)
if(NOT pybind11_FOUND)
  phylanx_error("pybind11 could not be found, please set pybind11_DIR to help locating it.")
endif()
if(${pybind11_VERSION} VERSION_LESS 2.2.0)
  phylanx_error("pybind11 too old, should be of version 2.2.0 or newer.")
endif()

phylanx_info("Python library version: " ${PYTHON_VERSION_STRING})
phylanx_info("Pybind11 library version: " ${pybind11_VERSION})

################################################################################
# Special compiler flags
if(WIN32)
  if(MSVC)
    # Display full paths in diagnostics
    phylanx_add_compile_flag(-FC LANGUAGES C CXX)

    phylanx_add_target_compile_option(-Ox CONFIGURATIONS Release)

    # even VS2017 has an ICE when compiling with -Ob2
    phylanx_add_target_compile_option(-Ob1 CONFIGURATIONS Release)

    if(NOT HPX_WITH_AWAIT)
      # /RTC1 is incompatible with /await
      phylanx_add_target_compile_option(/RTC1 CONFIGURATIONS Debug)
    else()
      phylanx_remove_target_compile_option(/RTC1 CONFIGURATIONS Debug)
    endif()

    # VS2012 and above has a special flag for improving the debug experience by
    # adding more symbol information to the build (-d2Zi)
    phylanx_add_target_compile_option(-d2Zi+ CONFIGURATIONS RelWithDebInfo)

    # VS2013 and above know how to do link time constant data segment folding
    # VS2013 update 2 and above know how to remove debug information for
    #     non-referenced functions and data (-Zc:inline)
    phylanx_add_target_compile_option(-Zc:inline)
    phylanx_add_target_compile_option(-Gw
      CONFIGURATIONS Release RelWithDebInfo MinSizeRelease)
    phylanx_add_target_compile_option(-Zo CONFIGURATIONS RelWithDebInfo)

    # Exceptions
    phylanx_add_target_compile_option(-EHsc)
    if(MSVC14)
      # assume conforming (throwing) operator new implementations
      phylanx_add_target_compile_option(-Zc:throwingNew)

      # enable faster linking (requires VS2015 Update1)
      # disabled for now as this flag crashes debugger
      # phylanx_remove_link_flag(/debug CONFIGURATIONS Debug)
      # phylanx_add_link_flag(/debug:fastlink CONFIGURATIONS Debug)

      # Update 3 allows to flag rvalue misuses and enforces strict string const-
      # qualification conformance
      phylanx_add_target_compile_option(-Zc:rvalueCast)
      phylanx_add_target_compile_option(-Zc:strictStrings)
    endif()

    # Runtime type information
    phylanx_add_target_compile_option(-GR)
    # Multiprocessor build
    phylanx_add_target_compile_option(-MP)
    # Increase the maximum size of object file sections
    phylanx_add_target_compile_option(-bigobj)
  endif()
endif()

if(NOT MSVC)
  # Show the flags that toggle each warning
  phylanx_add_compile_flag_if_available(-fdiagnostics-show-option LANGUAGES CXX C Fortran)

  # VLAs are a GNU extensions that we forbid as they are not supported on MSVC
  phylanx_add_compile_flag_if_available(-Werror=vla)
  # No return statement in a non-void function can lead to garbage return values
  # in GCC.
  phylanx_add_compile_flag_if_available(-Werror=return-type LANGUAGES CXX C)

  # We get false positives all over the place with this.
  if(CMAKE_COMPILER_IS_GNUCXX)
    phylanx_add_compile_flag_if_available(-Wno-unused-but-set-parameter LANGUAGES CXX C)
    phylanx_add_compile_flag_if_available(-Wno-unused-but-set-variable LANGUAGES CXX C)
    # Uninitialized variables are bad, earlier compilers issue spurious warnings
    # phylanx_add_compile_flag_if_available(-Werror=uninitialized LANGUAGES CXX C)
    phylanx_add_compile_flag_if_available(-Wno-unused-local-typedefs LANGUAGES CXX C)
  endif()

  # Silence warning about __sync_fetch_and_nand changing semantics
  phylanx_add_compile_flag_if_available(-Wno-sync-nand LANGUAGES CXX C)

  # Silence warnings about deleting polymorphic objects with non-virtual dtors.
  # These come from within Boost.
  if(CMAKE_COMPILER_IS_GNUCXX)
    phylanx_add_compile_flag_if_available(-Wno-delete-non-virtual-dtor LANGUAGES CXX)
  endif()

  # Check if our libraries have unresolved symbols
  #if(NOT APPLE AND NOT HPX_WITH_APEX)
  if(NOT APPLE AND NOT WIN32 AND NOT PHYLANX_WITH_SANITIZERS)
    phylanx_add_link_flag_if_available(-Wl,-z,defs TARGETS SHARED EXE)
  endif()

  if("${HPX_PLATFORM_UC}" STREQUAL "BLUEGENEQ")
    phylanx_add_compile_flag_if_available(-Wno-deprecated-register LANGUAGES CXX C)
  endif()

  if(PHYLANX_WITH_HIDDEN_VISIBILITY)
    phylanx_add_compile_flag_if_available(-fvisibility=hidden LANGUAGES CXX C Fortran)
    phylanx_add_link_flag_if_available(-fvisibility=hidden TARGETS SHARED EXE)
  endif()

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    phylanx_add_compile_flag_if_available(-Wno-cast-align)
  endif()

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # Disable the following warnings:
    # #1170: invalid redeclaration of nested class
    phylanx_add_compile_flag_if_available(-wd1170)
    # #858: type qualifier on return type is meaningless
    phylanx_add_compile_flag_if_available(-wd858)
    # #1098: the qualifier on this friend declaration is ignored
    phylanx_add_compile_flag_if_available(-wd1098)
    # #488: template parameter not used in declaring the parameter type
    phylanx_add_compile_flag_if_available(-wd488)
    # #2203: cast discards qualifiers from target type (needed for mvapich2
    #        mpi header)
    phylanx_add_compile_flag_if_available(-wd2203)
    # #2536: cannot specify explicit initializer for arrays
    phylanx_add_compile_flag_if_available(-wd2536)
  endif()
endif()

phylanx_include(SetOutputPaths)

################################################################################
# Build options

phylanx_option(
  PHYLANX_WITH_TESTS_BENCHMARKS BOOL
  "Enable or disable the compilation of benchmark tests"
  OFF ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_TESTS_REGRESSIONS BOOL
  "Enable or disable the compilation of regression tests"
  OFF ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_TESTS_UNIT BOOL
  "Enable or disable the compilation of unit tests"
  ON ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_EXAMPLES BOOL
  "Enable or disable the compilation of the examples"
  ON ADVANCED CATEGORY "Build")

if(MSVC)
  hpx_option(PHYLANX_WITH_PSEUDO_DEPENDENCIES BOOL
    "Force creating pseudo targets and pseudo dependencies (default OFF)."
    OFF CATEGORY "Build Targets")
else()
  hpx_option(PHYLANX_WITH_PSEUDO_DEPENDENCIES BOOL
    "Force creating pseudo targets and pseudo dependencies (default ON)."
    ON CATEGORY "Build Targets")
endif()

################################################################################
# Target specification

# Recurse into some subdirectories. This does not actually cause another cmake
# executable to run. The same process will walk through the project's entire
# directory structure.

add_phylanx_pseudo_target(core)
add_subdirectory(src)

if(PHYLANX_WITH_EXAMPLES)
  add_phylanx_pseudo_target(examples)
  include_directories(examples)
  add_subdirectory(examples)
endif()

###############################################################################
# Tests
if(PHYLANX_WITH_TESTS_BENCHMARKS OR PHYLANX_WITH_TESTS_REGRESSIONS OR PHYLANX_WITH_TESTS_UNIT)
  find_package(PythonInterp)

  if(NOT PYTHONINTERP_FOUND)
    phylanx_warn("A python interpreter could not be found. The test suite can not be run automatically.")
  endif()

  add_phylanx_pseudo_target(tests)

  enable_testing()
  include(CTest)

  if(NOT MSVC)
    add_custom_command(TARGET tests POST_BUILD
      COMMAND ctest --output-on-failure --timeout 100)
  endif()

  include_directories(tests)
  add_subdirectory(tests)
endif()

# Configure phylanxrun.py
configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/phylanxrun.py.in"
               "${CMAKE_BINARY_DIR}/bin/phylanxrun.py"
               @ONLY)

################################################################################
# installation instructions
################################################################################
install(
  FILES "${CMAKE_BINARY_DIR}/bin/phylanxrun.py"
  DESTINATION bin
  COMPONENT core
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
  GROUP_READ GROUP_EXECUTE
  WORLD_READ WORLD_EXECUTE)

# Install all Phylanx header files
install(
  DIRECTORY phylanx/
  DESTINATION include/phylanx
  COMPONENT core
  FILES_MATCHING PATTERN "*.hpp"
  PATTERN ".git" EXCLUDE
  PATTERN "CMakeFiles" EXCLUDE
  PATTERN "CTestFiles" EXCLUDE)

# Install all Phylanx cmake utility files
install(
  DIRECTORY cmake/
  DESTINATION ${LIB}/cmake/${PHYLANX_PACKAGE_NAME}
  COMPONENT core
  PATTERN ".in" EXCLUDE
  PATTERN ".git" EXCLUDE)

install(
  FILES "${PROJECT_SOURCE_DIR}/LICENSE_1_0.txt"
  DESTINATION share/phylanx-${PHYLANX_VERSION}
  COMPONENT license)
