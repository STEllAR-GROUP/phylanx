# Copyright (c) 2014 Thomas Heller
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

include(CMakePackageConfigHelpers)

set(CMAKE_DIR "cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}"
    CACHE STRING "directory (in share), where to put FindPhylanx cmake module")

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/${PHYLANX_PACKAGE_NAME}/${PHYLANX_PACKAGE_NAME}ConfigVersion.cmake"
  VERSION ${PHYLANX_VERSION}
  COMPATIBILITY AnyNewerVersion
)

export(TARGETS ${PHYLANX_EXPORT_TARGETS}
  FILE "${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/${PHYLANX_PACKAGE_NAME}/PhylanxTargets.cmake"
#  NAMESPACE phylanx::
)

# Get the include directories we need ...
get_directory_property(_INCLUDE_DIRS INCLUDE_DIRECTORIES)

# replace all characters with special regex meaning
set(special_chars "^;+;*;?;$;.;-;|;(;);]")
set(binarydir_escaped ${CMAKE_BINARY_DIR})
set(sourcedir_escaped ${PROJECT_SOURCE_DIR})
foreach(special_char ${special_chars})
  string(REPLACE "${special_char}" "\\${special_char}" binarydir_escaped ${binarydir_escaped})
  string(REPLACE "${special_char}" "\\${special_char}" sourcedir_escaped ${sourcedir_escaped})
endforeach()

# '[' has special meaning in lists
string(REPLACE "[" "\\[" binarydir_escaped ${binarydir_escaped})
string(REPLACE "[" "\\[" sourcedir_escaped ${sourcedir_escaped})

foreach(dir ${_INCLUDE_DIRS})
  if((NOT dir MATCHES "^${binarydir_escaped}.*")
    AND (NOT dir MATCHES "^${sourcedir_escaped}.*"))
    set(_NEEDED_INCLUDE_DIRS "${_NEEDED_INCLUDE_DIRS} -I${dir}")
    set(_NEEDED_CMAKE_INCLUDE_DIRS ${_NEEDED_CMAKE_INCLUDE_DIRS} "${dir}")
  else()
    set(_NEEDED_BUILD_DIR_INCLUDE_DIRS "${_NEEDED_BUILD_DIR_INCLUDE_DIRS} -I${dir}")
    set(_NEEDED_CMAKE_BUILD_DIR_INCLUDE_DIRS ${_NEEDED_CMAKE_BUILD_DIR_INCLUDE_DIRS} "${dir}")
  endif()
endforeach()

# Configure config for the install dir ...
set(BLAZE_CONF_DIR ${blaze_DIR})
set(HPX_CONF_DIR ${HPX_DIR})

set(PHYLANX_CONF_PREFIX ${CMAKE_INSTALL_PREFIX})
set(PHYLANX_CONF_LIBRARIES "general;phylanx_component")
set(PHYLANX_CONF_LIBRARY_DIR ${PHYLANX_LIBRARY_DIR})
set(PHYLANX_CONF_INCLUDE_DIRS
  "-I${CMAKE_INSTALL_PREFIX}/include ${_NEEDED_INCLUDE_DIRS}")
set(PHYLANX_CMAKE_CONF_INCLUDE_DIRS
  ${CMAKE_INSTALL_PREFIX}/include
  ${_NEEDED_CMAKE_INCLUDE_DIRS})

configure_file(cmake/templates/${PHYLANX_PACKAGE_NAME}Config.cmake.in
  "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PHYLANX_PACKAGE_NAME}Config.cmake"
  ESCAPE_QUOTES @ONLY)

# ... and the build dir
set(PHYLANX_CONF_PREFIX ${CMAKE_BINARY_DIR})
set(PHYLANX_CONF_LIBRARIES "general;phylanx_component")
set(PHYLANX_CONF_LIBRARY_DIR ${CMAKE_BINARY_DIR}/lib)
set(PHYLANX_CONF_INCLUDE_DIRS
  "${_NEEDED_BUILD_DIR_INCLUDE_DIRS} ${_NEEDED_INCLUDE_DIRS}")
set(PHYLANX_CMAKE_CONF_INCLUDE_DIRS
  ${_NEEDED_CMAKE_BUILD_DIR_INCLUDE_DIRS}
  ${_NEEDED_CMAKE_INCLUDE_DIRS})

configure_file(cmake/templates/${PHYLANX_PACKAGE_NAME}Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/${PHYLANX_PACKAGE_NAME}/${PHYLANX_PACKAGE_NAME}Config.cmake"
  ESCAPE_QUOTES @ONLY)

# Configure macros for the install dir ...
set(PHYLANX_CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/cmake/${PHYLANX_PACKAGE_NAME}")
configure_file(cmake/templates/PhylanxMacros.cmake.in
  "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PhylanxMacros.cmake"
  ESCAPE_QUOTES @ONLY)

# ... and the build dir
set(PHYLANX_CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
configure_file(cmake/templates/PhylanxMacros.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/${PHYLANX_PACKAGE_NAME}/PhylanxMacros.cmake"
  ESCAPE_QUOTES @ONLY)

install(
  EXPORT PhylanxTargets
  FILE PhylanxTargets.cmake
#  NAMESPACE phylanx::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PHYLANX_PACKAGE_NAME}
)

install(
  FILES
    "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PHYLANX_PACKAGE_NAME}Config.cmake"
    "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PhylanxMacros.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/${PHYLANX_PACKAGE_NAME}/${PHYLANX_PACKAGE_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PHYLANX_PACKAGE_NAME}
  COMPONENT cmake
)

