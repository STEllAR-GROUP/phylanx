# Copyright (c) 2014      Thomas Heller
# Copyright (c) 2007-2017 Hartmut Kaiser
# Copyright (c) 2011      Bryce Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_policy(PUSH)
phylanx_set_cmake_policy(SET CMP0054 NEW)

function(phylanx_setup_target target)
  # retrieve arguments
  set(options EXPORT NOHPX_INIT INSTALL NOLIBS PLUGIN NONAMEPREFIX)
  set(one_value_args TYPE FOLDER NAME SOVERSION VERSION PHYLANX_PREFIX)
  set(multi_value_args DEPENDENCIES COMPONENT_DEPENDENCIES COMPILE_FLAGS LINK_FLAGS INSTALL_FLAGS)
  cmake_parse_arguments(target "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  phylanx_is_target(is_target ${target})
  if(NOT is_target)
    phylanx_error("${target} does not represent a target")
  endif()

  # Figure out which type we want...
  if(target_TYPE)
    string(TOUPPER "${target_TYPE}" _type)
  else()
    get_target_property(type_prop ${target} TYPE)
    if(type_prop STREQUAL "STATIC_LIBRARY")
      set(_type "LIBRARY")
    endif()
    if(type_prop STREQUAL "MODULE_LIBRARY")
      set(_type "LIBRARY")
    endif()
    if(type_prop STREQUAL "SHARED_LIBRARY")
      set(_type "LIBRARY")
    endif()
    if(type_prop STREQUAL "EXECUTABLE")
      set(_type "EXECUTABLE")
    endif()
  endif()

  if(target_FOLDER)
    set_target_properties(${target} PROPERTIES FOLDER "${target_FOLDER}")
  endif()

  get_target_property(target_SOURCES ${target} SOURCES)

  # Manage files with .cu extension in case When Cuda Clang is used
  if(target_SOURCES AND PHYLANX_WITH_CUDA_CLANG)
    foreach(source ${target_SOURCES})
      get_filename_component(extension ${source} EXT)
    endforeach()
  endif()

  if(target_COMPILE_FLAGS)
    phylanx_append_property(${target} COMPILE_FLAGS ${target_COMPILE_FLAGS})
  endif()

  if(target_LINK_FLAGS)
    phylanx_append_property(${target} LINK_FLAGS ${target_LINK_FLAGS})
  endif()

  if(target_NAME)
    set(name "${target_NAME}")
  else()
    set(name "${target}")
  endif()

  set(target_STATIC_LINKING OFF)
  if(PHYLANX_WITH_STATIC_LINKING)
    set(target_STATIC_LINKING ON)
  else()

    set(_phylanx_library_type)
    if(TARGET phylanx)
      get_target_property(_phylanx_library_type phylanx TYPE)
    endif()

    if("${_phylanx_library_type}" STREQUAL "STATIC_LIBRARY")
      set(target_STATIC_LINKING ON)
    endif()
  endif()

  if(PHYLANX_INCLUDE_DIRS)
    set_property(
      TARGET ${target} APPEND
      PROPERTY INCLUDE_DIRECTORIES
      "${PHYLANX_INCLUDE_DIRS}")
  endif()

  if("${_type}" STREQUAL "EXECUTABLE")
    if(target_PHYLANX_PREFIX)
      set(_prefix ${target_PHYLANX_PREFIX})
    else()
      set(_prefix ${PHYLANX_PREFIX})
    endif()

    if(MSVC)
      string(REPLACE ";" ":" _prefix "${_prefix}")
    endif()

    set_property(TARGET ${target} APPEND
                 PROPERTY COMPILE_DEFINITIONS
                 "PHYLANX_APPLICATION_NAME=${name}"
                 "PHYLANX_APPLICATION_STRING=\"${name}\""
                 "PHYLANX_APPLICATION_EXPORTS"
                 "HPX_APPLICATION_EXPORTS")
  endif()

  if("${_type}" STREQUAL "LIBRARY")
    if(DEFINED PHYLANX_LIBRARY_VERSION AND DEFINED PHYLANX_SOVERSION)
      # set properties of generated shared library
      set_target_properties(${target}
        PROPERTIES
        VERSION ${PHYLANX_LIBRARY_VERSION}
        SOVERSION ${PHYLANX_SOVERSION})
    endif()
    if(NOT target_NONAMEPREFIX)
      phylanx_set_lib_name(${target} ${name})
    endif()
    set_target_properties(${target}
      PROPERTIES
      # create *nix style library versions + symbolic links
      # allow creating static and shared libs without conflicts
      CLEAN_DIRECT_OUTPUT 1
      OUTPUT_NAME ${name})

    set_property(TARGET ${target} APPEND
                 PROPERTY COMPILE_DEFINITIONS
                 "PHYLANX_LIBRARY_EXPORTS")
  endif()

  if("${_type}" STREQUAL "PRIMITIVE")

    if(NOT target_NONAMEPREFIX)
      phylanx_set_lib_name(${target} ${name})
    endif()
    set_target_properties(${target}
      PROPERTIES
      # create *nix style library versions + symbolic links
      # allow creating static and shared libs without conflicts
      CLEAN_DIRECT_OUTPUT 1
      OUTPUT_NAME ${name})

    set_property(TARGET ${target}
      APPEND PROPERTY
        COMPILE_DEFINITIONS
          "HPX_PLUGIN_NAME=phylanx_${name}"
          "HPX_PLUGIN_STRING=\"phylanx_${name}\""
          "HPX_LIBRARY_EXPORTS"
          "PHYLANX_LIBRARY_EXPORTS")

  endif()

  # if Phylanx is an imported target, get the config debug/release
  set(PHYLANX_IMPORT_CONFIG "NOTFOUND")
  if (TARGET "phylanx")
    get_target_property(PHYLANX_IMPORT_CONFIG "phylanx" IMPORTED_CONFIGURATIONS)
  endif()
  if(PHYLANX_IMPORT_CONFIG MATCHES NOTFOUND)
    # we are building Phylanx not importing, so we should use the $<CONFIG:variable
    set(_USE_CONFIG 1)
  else()
    # Phylanx is an imported target, so set PHYLANX_DEBUG based on build config of
    # Phylanx library
    set(_USE_CONFIG 0)
  endif()

  # linker instructions
  if(TARGET blaze::blaze)
    set(phylanx_libs blaze::blaze)
  endif()
  if(TARGET BlazeTensor::BlazeTensor)
    set(phylanx_libs BlazeTensor::BlazeTensor)
  endif()

  if(NOT target_NOLIBS)
    set(phylanx_libs ${phylanx_libs} "general;phylanx_component")
#    if(NOT target_STATIC_LINKING)
#      set(phylanx_libs ${phylanx_libs})
#    endif()
    phylanx_handle_component_dependencies(target_COMPONENT_DEPENDENCIES)
    set(phylanx_libs ${phylanx_libs} ${target_COMPONENT_DEPENDENCIES})
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
      set(phylanx_libs ${phylanx_libs} imf svml irng intlc)
    endif()
    if(DEFINED PHYLANX_LIBRARIES)
      set(phylanx_libs ${phylanx_libs} ${PHYLANX_LIBRARIES})
    endif()
    if(HPX_WITH_DYNAMIC_HPX_MAIN AND ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux") AND ("${_type}" STREQUAL "EXECUTABLE"))
      set_target_properties(${target} PROPERTIES LINK_FLAGS "${HPX_LINKER_FLAGS}")
      set(phylanx_libs "${HPX_LINK_LIBRARIES}${phylanx_libs}")
    endif()
  else()
    target_compile_options(${target} PUBLIC ${CXX_FLAG})
  endif()

  phylanx_debug("phylanx_setup_target.${target}: phylanx_libs:" ${phylanx_libs})
  target_link_libraries(${target} ${PHYLANX_TLL_PUBLIC} ${phylanx_libs} ${target_DEPENDENCIES})

  get_target_property(target_EXCLUDE_FROM_ALL ${target} EXCLUDE_FROM_ALL)

  if(target_EXPORT AND NOT target_EXCLUDE_FROM_ALL)
    phylanx_export_targets(${target})
    set(install_export EXPORT PhylanxTargets)
  endif()

  if(target_INSTALL AND NOT target_EXCLUDE_FROM_ALL)
    install(TARGETS ${target} ${target_INSTALL_FLAGS} ${install_export})
  endif()
endfunction()

cmake_policy(POP)
