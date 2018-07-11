# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup HPX as a dependency
macro(phylanx_setup_hpx)
  if(NOT HPX_DIR AND HPX_ROOT)
    set(HPX_DIR ${HPX_ROOT}/lib/cmake/HPX)
  endif()

  if(NOT HPX_DIR AND EXISTS "$ENV{HPX_DIR}")
    set(HPX_DIR $ENV{HPX_DIR})
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

    if (HPX_GIT_COMMIT)
      string(SUBSTRING ${HPX_GIT_COMMIT} 0 10 __hpx_git_commit)
      phylanx_info("HPX version: " ${HPX_VERSION_STRING} "(${__hpx_git_commit})")
    else()
      phylanx_info("HPX version: " ${HPX_VERSION_STRING})
    endif()

    # make sure that HPX is not configured with jemalloc
    if(${HPX_WITH_MALLOC} STREQUAL "jemalloc")
        phylanx_warn(
          "HPX is configured with: ${HPX_WITH_MALLOC}. Due to incompatibilities between the Python runtime and jemalloc, application execution will fail unless the jemalloc library is preloaded with LD_PRELOAD. For more reliable execution, we recommend reconfiguring HPX and Phylanx with TCMalloc")
    endif()

  else()
    phylanx_error("HPX_DIR has not been specified, please set it to help locating HPX")
  endif()
endmacro()
