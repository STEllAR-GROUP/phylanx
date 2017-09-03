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
  GLOB GLOBS "${PROJECT_SOURCE_DIR}/src/python/*.cpp")

add_phylanx_source_group(
  NAME phylanx_py
  CLASS "Source Files"
  ROOT "${PROJECT_SOURCE_DIR}/src/python"
  TARGETS ${phylanx_SOURCES})

if(MSVC)
  # libphylanx_py headers
  add_phylanx_library_headers(phylanx_py
    GLOB GLOBS "${PROJECT_SOURCE_DIR}/phylanx/python/*.hpp")

  add_phylanx_source_group(
    NAME phylanx_py
    CLASS "Header Files"
    ROOT "${PROJECT_SOURCE_DIR}/phylanx/python"
    TARGETS ${phylanx_HEADERS})
endif()

###############################################################################
pybind11_add_module(phylanx_py ${phylanx_py_SOURCES} ${phylanx_py_HEADERS})

include_directories(${pybind11_INCLUDE_DIR}})

target_link_libraries(phylanx_py PRIVATE phylanx ${pybind11_LIBRARIES})
target_compile_definitions(phylanx_py PRIVATE NOMINMAX)

set_target_properties(
  phylanx_py PROPERTIES
    VERSION ${PHYLANX_VERSION}
    SOVERSION ${PHYLANX_SOVERSION}
    CLEAN_DIRECT_OUTPUT 1
    OUTPUT_NAME phylanx
    FOLDER "Core")

if(MSVC)
  set_target_properties(
    phylanx_py PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/bin"
      LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/bin"
      LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/MinSizeRel/bin"
      LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/RelWithDebInfo/bin")
else()
  set_target_properties(
    phylanx_py PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

set(phylanx_targets ${phylanx_targets} phylanx_py)

