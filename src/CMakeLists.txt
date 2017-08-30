# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

################################################################################
set(phylanx_SOURCES "" CACHE INTERNAL "Sources for libphylanx." FORCE)
if(MSVC)
  set(phylanx_HEADERS "" CACHE INTERNAL "Headers for libphylanx." FORCE)
endif()

################################################################################
# gather sources/headers for main library

# libphylanx sources
add_phylanx_library_sources(phylanx
  GLOB GLOBS "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_phylanx_library_sources(phylanx
  GLOB GLOBS "${PROJECT_SOURCE_DIR}/src/ir/*.cpp"
  APPEND)

add_phylanx_source_group(
  NAME phylanx
  CLASS "Source Files"
  ROOT "${PROJECT_SOURCE_DIR}/src"
  TARGETS ${phylanx_SOURCES})

if(MSVC)
  # libphylanx headers
  add_phylanx_library_headers(phylanx
    GLOB GLOBS "${PROJECT_SOURCE_DIR}/phylanx/*.hpp")
  add_phylanx_library_headers(phylanx
    GLOB GLOBS "${PROJECT_SOURCE_DIR}/phylanx/config/*.hpp"
    APPEND)
  add_phylanx_library_headers(phylanx
    GLOB GLOBS "${PROJECT_SOURCE_DIR}/phylanx/ast/*.hpp"
    APPEND)
  add_phylanx_library_headers(phylanx
    GLOB GLOBS "${PROJECT_SOURCE_DIR}/phylanx/ir/*.hpp"
    APPEND)
  add_phylanx_library_headers(phylanx
    GLOB_RECURSE GLOBS "${PROJECT_SOURCE_DIR}/phylanx/util/*.hpp"
    APPEND)
  add_phylanx_library_headers(phylanx
    GLOB GLOBS "${PROJECT_SOURCE_DIR}/phylanx/include/*.hpp"
    APPEND)

  add_phylanx_source_group(
    NAME phylanx
    CLASS "Header Files"
    ROOT "${PROJECT_SOURCE_DIR}/phylanx"
    TARGETS ${phylanx_HEADERS})
endif()

################################################################################
# create main module
add_library(phylanx SHARED ${phylanx_SOURCES} ${phylanx_HEADERS})

target_link_libraries(phylanx ${HPX_LIBRARIES})

set_target_properties(
  phylanx PROPERTIES
    VERSION ${PHYLANX_VERSION}
    SOVERSION ${PHYLANX_SOVERSION}
    CLEAN_DIRECT_OUTPUT 1
    OUTPUT_NAME phylanx
    FOLDER "Core")

set_property(TARGET phylanx APPEND
  PROPERTY COMPILE_DEFINITIONS
  "HPX_COMPONENT_NAME=phylanx"
  "PHYLANX_EXPORTS")

set(phylanx_targets ${phylanx_targets} phylanx)

install(
  TARGETS ${phylanx_targets}
  EXPORT PhylanxTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  COMPONENT runtime
  OPTIONAL)

phylanx_export_targets(${phylanx_targets})

foreach(target ${phylanx_targets})
  add_phylanx_pseudo_dependencies(core ${target})
endforeach()
