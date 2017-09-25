# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup Eigen3 as a dependency
macro(phylanx_setup_eigen3)

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

endmacro()
