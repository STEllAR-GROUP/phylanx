# Copyright (c) 2008 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

macro(add_phylanx_primitive_plugin name)
  # retrieve arguments
  set(options EXCLUDE_FROM_ALL AUTOGLOB)
  set(one_value_args INI FOLDER SOURCE_ROOT HEADER_ROOT SOURCE_GLOB HEADER_GLOB)
  set(multi_value_args SOURCES HEADERS DEPENDENCIES COMPONENT_DEPENDENCIES COMPILE_FLAGS LINK_FLAGS)
  cmake_parse_arguments(${name} "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  phylanx_debug("Add primitive plugin ${name}: ${name}_COMPONENT_DEPENDENCIES: ${${name}_COMPONENT_DEPENDENCIES}")
  phylanx_debug("Add primitive plugin ${name}: ${name}_COMPONENT_DEPENDENCIES: ${${name}_COMPONENT_DEPENDENCIES}")
  phylanx_debug("Add primitive plugin ${name}: ${name}_FOLDER: ${${name}_FOLDER}")
  phylanx_debug("Add primitive plugin ${name}: ${name}_HEADER_ROOT: ${${name}_HEADER_ROOT}")
  phylanx_debug("Add primitive plugin ${name}: ${name}_SOURCE_ROOT: ${${name}_SOURCE_ROOT}")

  set(__autoglob)
  if(${name}_AUTOGLOB)
    set(__autoglob AUTOGLOB)
  endif()

  add_hpx_component(${name}
    INI ${${name}_INI}
    FOLDER ${${name}_FOLDER}
    HEADER_ROOT ${${name}_HEADER_ROOT}
    SOURCE_ROOT ${${name}_SOURCE_ROOT}
    HEADER_GLOB ${${name}_HEADER_GLOB}
    SOURCE_GLOB ${${name}_SOURCE_GLOB}
    HEADERS ${${name}_HEADERS}
    SOURCES ${${name}_SOURCES}
    COMPILE_FLAGS ${${name}_COMPILE_FLAGS}
    LINK_FLAGS ${${name}_LINK_FLAGS}
    ${__autoglob}
    DEPENDENCIES ${${name}_DEPENDENCIES}
    COMPONENT_DEPENDENCIES ${${name}_COMPONENT_DEPENDENCIES}
    OUTPUT_SUFFIX phylanx
    INSTALL_SUFFIX phylanx)

  target_link_libraries(${name}_component
    PUBLIC
      ${HPX_LIBRARIES}
      ${BLAS_LIBRARIES}
      ${LAPACK_LIBRARIES})
  target_link_libraries(${name}_component
    PRIVATE
      blaze::blaze)

  set_property(TARGET ${name}_component
    APPEND PROPERTY
      COMPILE_DEFINITIONS
        "PHYLANX_PLUGIN_EXPORTS"
        "HPX_PLUGIN_NAME=hpx_${name}")

endmacro()

