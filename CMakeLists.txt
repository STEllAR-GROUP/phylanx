# Copyright (c) 2017-2018 Hartmut Kaiser
# Copyright (c) 2018 R. Tohid
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
project(Phylanx C CXX)

set(PHYLANX_MAJOR_VERSION 0)
set(PHYLANX_MINOR_VERSION 0)
set(PHYLANX_PATCH_LEVEL   1)

set(PHYLANX_VERSION "${PHYLANX_MAJOR_VERSION}.${PHYLANX_MINOR_VERSION}.${PHYLANX_PATCH_LEVEL}")
set(PHYLANX_LIBRARY_VERSION "${PHYLANX_VERSION}")
set(PHYLANX_SOVERSION ${PHYLANX_MAJOR_VERSION})
set(PHYLANX_PACKAGE_NAME Phylanx)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

include(GNUInstallDirs)
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
# Special compiler flags
phylanx_setup_compiler_flags()
phylanx_setup_output_paths()
phylanx_detect_cpp_dialect()

################################################################################
# Build options
phylanx_option(
  PHYLANX_WITH_TASK_INLINING_POLICY BOOL
  "Enable or disable the adaptive task inlining policy"
  OFF ADVANCED CATEGORY "Build")

if(PHYLANX_WITH_TASK_INLINING_POLICY)
  phylanx_add_config_define(PHYLANX_HAVE_TASK_INLINING_POLICY)
  message("Adaptive task inlining policy enabled.")
endif()

phylanx_option(
  PHYLANX_WITH_TESTS_BENCHMARKS BOOL
  "Enable or disable the compilation of benchmark tests"
  ON ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_TESTS_REGRESSIONS BOOL
  "Enable or disable the compilation of regression tests"
  ON ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_TESTS_UNIT BOOL
  "Enable or disable the compilation of unit tests"
  ON ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_EXAMPLES BOOL
  "Enable or disable the compilation of the examples"
  ON ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_DOCUMENTATION BOOL
  "Enable or disable building Documentation"
  OFF ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_TOOLS BOOL
  "Enable or disable building tools"
  OFF ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_HIGHFIVE BOOL
  "Enable or disable building HDF5 Support"
  OFF ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_VIM_YCM BOOL
  "Enable or disable YouCompleteMe configuration support for VIM"
  OFF ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_ITERATIVE_SOLVERS BOOL
  "Enable or disable the Blaze iterative solvers"
  OFF ADVANCED CATEGORY "Build")

phylanx_option(
  PHYLANX_WITH_BLAZE_TENSOR BOOL
  "Enable or disable Blaze tensor types"
  OFF ADVANCED CATEGORY "Build")

if(MSVC)
  phylanx_option(PHYLANX_WITH_PSEUDO_DEPENDENCIES BOOL
    "Force creating pseudo targets and pseudo dependencies (default OFF)."
    OFF CATEGORY "Build Targets")
else()
  phylanx_option(PHYLANX_WITH_PSEUDO_DEPENDENCIES BOOL
    "Force creating pseudo targets and pseudo dependencies (default ON)."
    ON CATEGORY "Build Targets")
endif()

# Locate dependencies
phylanx_setup_blaze()
phylanx_setup_pybind11()
phylanx_setup_highfive()
phylanx_setup_hpx()

phylanx_include(GitCommit)

###############################################################################
if(NOT HPX_WITH_CUDA)
  set(HPX_TLL_PUBLIC "PUBLIC")      # keywords for target_link_libraries
  set(HPX_TLL_PRIVATE "PRIVATE")
endif()

set(PHYLANX_TLL_PUBLIC ${HPX_TLL_PUBLIC})
set(PHYLANX_TLL_PRIVATE ${HPX_TLL_PRIVATE})

################################################################################
# Set basic search paths for the generated Phylanx headers
################################################################################
include_directories("${CMAKE_BINARY_DIR}")

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

################################################################################
# Targets related to Python (must come before tests)
add_subdirectory(python)

phylanx_export_targets(${phylanx_targets})

foreach(target ${phylanx_targets})
  add_phylanx_pseudo_dependencies(core ${target})
endforeach()

###############################################################################
# Must come after Python
if(PHYLANX_WITH_DOCUMENTATION)
  add_subdirectory(docs)
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
    find_program(CTEST_EXECUTABLE "ctest")
    add_custom_command(TARGET tests POST_BUILD
      COMMAND ${CTEST_EXECUTABLE} --output-on-failure --timeout 100 -C $<CONFIG>)
    set_target_properties(tests
      PROPERTIES FOLDER "Tests")
  endif()

  include_directories(tests)
  add_subdirectory(tests)
endif()

# Configure phylanxrun.py
if(HPX_WITH_PARCELPORT_MPI)
    find_package(MPI QUIET)
endif(HPX_WITH_PARCELPORT_MPI)
configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/phylanxrun.py.in"
               "${CMAKE_BINARY_DIR}/bin/phylanxrun.py"
               @ONLY)

################################################################################
# Targets related to Tools
if(PHYLANX_WITH_TOOLS)
  add_phylanx_pseudo_target(tools)
  add_subdirectory(tools)
endif()

################################################################################
# Configure the header to include all compile definitions
################################################################################

################################################################################
# PHYLANX_PREFIX
# The prefix is the default search path for Phylanx plugins
################################################################################
if("${PHYLANX_PLATFORM_UC}" STREQUAL "ANDROID")
  set(PHYLANX_PREFIX "lib")
  set(PHYLANX_BUILD_PREFIX "lib")
else()
  set(PHYLANX_PREFIX "${CMAKE_INSTALL_PREFIX}")
  set(PHYLANX_BUILD_PREFIX "${CMAKE_BINARY_DIR}")
endif()

# Generate a defines.hpp to be used in the build directory ...
set(PHYLANX_DEFINES_PREFIX ${PHYLANX_BUILD_PREFIX})
phylanx_write_config_defines_file(
  TEMPLATE "${PROJECT_SOURCE_DIR}/cmake/templates/config_defines.hpp.in"
  NAMESPACE default
  FILENAME "${CMAKE_BINARY_DIR}/phylanx/config/defines.hpp")

# Generate a defines.hpp to be used in the install directory ...
set(PHYLANX_DEFINES_PREFIX ${PHYLANX_PREFIX})
phylanx_write_config_defines_file(
  TEMPLATE "${PROJECT_SOURCE_DIR}/cmake/templates/config_defines.hpp.in"
  NAMESPACE default
  FILENAME "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/phylanx/config/defines.hpp")

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

# Install all Phylanx header that have been configured using various
# cmake options
install(
  DIRECTORY "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/phylanx/"
  DESTINATION include/phylanx
  COMPONENT core
  FILES_MATCHING PATTERN "*.hpp"
  PATTERN ".git" EXCLUDE
  PATTERN "CMakeFiles" EXCLUDE
  PATTERN "CTestFiles" EXCLUDE)

# Install all Phylanx cmake utility files
install(
  DIRECTORY cmake/
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PHYLANX_PACKAGE_NAME}
  COMPONENT core
  PATTERN ".in" EXCLUDE
  PATTERN ".git" EXCLUDE)

install(
  FILES "${PROJECT_SOURCE_DIR}/LICENSE_1_0.txt"
  DESTINATION share/phylanx-${PHYLANX_VERSION}
  COMPONENT license)

# install phylanx docs
if(PHYLANX_WITH_DOCUMENTATION)
  # Install Phylanx documentation files
  install(
    FILES "${PROJECT_SOURCE_DIR}/docs/index.html"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/phylanx/ OPTIONAL
    COMPONENT docs)

  install(
    DIRECTORY "${CMAKE_BINARY_DIR}/share/phylanx/docs/html/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/phylanx/docs/html OPTIONAL
    COMPONENT docs
    PATTERN ".git" EXCLUDE)
endif()

phylanx_include(GeneratePackage)

# Create a symlink in share pointing to the latest Phylanx installation
# create_symbolic_link("${CMAKE_BINARY_DIR}/share/phylanx-${PHYLANX_VERSION}"
#                      "${CMAKE_BINARY_DIR}/share/phylanx")

message("")
message("Phylanx will be installed to ${CMAKE_INSTALL_PREFIX}")
message("")
