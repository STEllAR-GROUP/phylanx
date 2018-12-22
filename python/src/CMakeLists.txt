# Copyright (c) 2014-2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

###############################################################################
set(phylanx_py_SOURCES "" CACHE INTERNAL "Sources for libphylanx_py." FORCE)
if(MSVC)
  set(phylanx_py_HEADERS "" CACHE INTERNAL "Headers for libphylanx_py." FORCE)
endif()

add_phylanx_library_sources(phylanx_py
  GLOB_RECURSE GLOBS "${PROJECT_SOURCE_DIR}/python/src/*.cpp")

add_phylanx_source_group(
  NAME phylanx_py
  CLASS "Source Files"
  ROOT "${PROJECT_SOURCE_DIR}/python/src"
  TARGETS ${phylanx_py_SOURCES})

add_hpx_library_headers(phylanx_py
  GLOB_RECURSE GLOBS "${PROJECT_SOURCE_DIR}/python/src/*.hpp")

add_phylanx_source_group(
  NAME phylanx_py
  CLASS "Header Files"
  ROOT "${PROJECT_SOURCE_DIR}/python/src"
  TARGETS ${phylanx_py_HEADERS})

# create pseudo project holding all Phylanx Python files
add_phylanx_library_sources(phylanx_python
  GLOB GLOBS "${PROJECT_SOURCE_DIR}/python/phylanx/*.py")
add_phylanx_library_sources(phylanx_python
  GLOB GLOBS "${PROJECT_SOURCE_DIR}/python/phylanx/ast/*.py"
  APPEND)
add_phylanx_library_sources(phylanx_python
  GLOB GLOBS "${PROJECT_SOURCE_DIR}/python/phylanx/core/*.py"
  APPEND)
add_phylanx_library_sources(phylanx_python
  GLOB GLOBS "${PROJECT_SOURCE_DIR}/python/phylanx/execution_tree/*.py"
  APPEND)
add_phylanx_library_sources(phylanx_python
  GLOB GLOBS "${PROJECT_SOURCE_DIR}/python/phylanx/ir/*.py"
  APPEND)
add_phylanx_library_sources(phylanx_python
  GLOB GLOBS "${PROJECT_SOURCE_DIR}/python/phylanx/util/*.py"
  APPEND)

add_phylanx_source_group(
  NAME phylanx_py
  CLASS "Python Files"
  ROOT "${PROJECT_SOURCE_DIR}/python"
  TARGETS ${phylanx_python_SOURCES})

###############################################################################
if(APPLE) # OSX requires this to be a shared library, not a module (default)
  pybind11_add_module(phylanx_py
    SHARED
      ${phylanx_py_SOURCES}
      ${phylanx_py_HEADERS})
else(APPLE)
  pybind11_add_module(phylanx_py
    ${phylanx_py_SOURCES}
    ${phylanx_py_HEADERS}
    ${phylanx_python_SOURCES})
endif(APPLE)

include_directories(${pybind11_INCLUDE_DIR})

# this makes sure CMake uses the proper HPX library dependencies
target_link_libraries(phylanx_py
  ${HPX_TLL_PRIVATE}
    phylanx_component
    blaze::blaze
    ${pybind11_LIBRARIES})
if(PHYLANX_WITH_BLAZE_TENSOR)
  target_link_libraries(phylanx_py
    ${HPX_TLL_PRIVATE}
      BlazeTensor::BlazeTensor)
endif()
if(NOT MSVC)
    foreach(dir ${HPX_LIBRARY_DIR})
        target_link_libraries(phylanx_py PRIVATE -L${dir})
    endforeach()
endif()

target_include_directories(phylanx_py
  PRIVATE
    ${pybind11_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/python/src")

set_target_properties(phylanx_py
  PROPERTIES
    VERSION ${PHYLANX_VERSION}
    SOVERSION ${PHYLANX_SOVERSION}
    CLEAN_DIRECT_OUTPUT 1
    OUTPUT_NAME _phylanx
    FOLDER "Core")

if(MSVC)
  set_target_properties(phylanx_py
    PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/bin"
      LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/bin"
      LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/MinSizeRel/bin"
      LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/RelWithDebInfo/bin")
else()
  set_target_properties(phylanx_py
    PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

set(phylanx_targets ${phylanx_targets} phylanx_py PARENT_SCOPE)

