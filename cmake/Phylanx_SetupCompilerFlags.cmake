# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Set special compiler flags
macro(phylanx_setup_compiler_flags)

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
      if(NOT (${MSVC_VERSION} LESS 1900))
        # assume conforming (throwing) operator new implementations
        phylanx_add_target_compile_option(/Zc:throwingNew)

        # enable faster linking (requires VS2015 Update1)
        # disabled for now as this flag crashes debugger
        # phylanx_remove_link_flag(/debug CONFIGURATIONS Debug)
        # phylanx_add_link_flag(/debug:fastlink CONFIGURATIONS Debug)

        # Update 3 allows to flag rvalue misuses and enforces strict string const-
        # qualification conformance
        phylanx_add_target_compile_option(/Zc:rvalueCast)
        phylanx_add_target_compile_option(/Zc:strictStrings)
      endif()

      # Suppress Blaze warnings
      # Description: Without suppressing them MSVC generates a plethora of warnings
      # that have nothing to do with HPX and Phylanx
      # warning C4146: unary minus operator applied to unsigned type, result still unsigned
      phylanx_add_compile_flag(/wd4146)
      # warning C4244: conversion from 'uint64_t' to 'int', possible loss of data
      phylanx_add_compile_flag(/wd4244)
      # warning C4167: conversion from 'size_t' to 'int', possible loss of data
      phylanx_add_compile_flag(/wd4267)

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

endmacro()
