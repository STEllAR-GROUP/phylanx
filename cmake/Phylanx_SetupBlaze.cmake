# Copyright (c) 2017 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup Blaze as a dependency

# TODO: We don't use `phylanx_setup_eigen3`. We can remove `Phylanx_SetupEigen3.cmake`.

macro(phylanx_setup_blaze)
  # Method 2: Specify the include directory `blaze_INCLUDE_DIR`
  if(blaze_INCLUDE_DIR)
    message("Adding defined blaze_INCLUDE_DIR to header include directories.")
    list(APPEND CMAKE_REQUIRED_INCLUDES ${blaze_INCLUDE_DIR})
    include_directories(${blaze_INCLUDE_DIR})
  endif()
  # Method 3: Vcpkg
  if(_VCPKG_ROOT_DIR)
    message("Vcpkg found. Adding to header include directory: " ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)
    list(APPEND CMAKE_REQUIRED_INCLUDES "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
    include_directories("${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
  endif()

  # Check if Math.h is available
  include(CheckIncludeFileCXX)
  if(MSCVC)
    set(CMAKE_REQUIRED_FLAGS "/std:c++14")
  else()
    set(CMAKE_REQUIRED_FLAGS "-std=c++14")
  endif()
  check_include_file_cxx(blaze/Math.h HAVE_BLAZE_MATH_H)
  if(NOT HAVE_BLAZE_MATH_H)
    # Method 1: find_package
    # NOTE: `blaze_DIR` being set assists this step.
    find_package(blaze NO_CMAKE_PACKAGE_REGISTRY)
    if(blaze_FOUND)
      add_library(blaze_target INTERFACE)
      target_link_libraries(blaze_target INTERFACE blaze::blaze)
    else()
      # HACK: PHYLANX_SKIP_BLAZE_CHECK
      # - If Blaze is available through vcpkg, vcpkg's toolchain script does
      #   not offer any variables to detect blaze's location when this script
      #   is running.
      if(NOT PHYLANX_SKIP_BLAZE_CHECK)
        phylanx_error("Blaze could not be found. Please set blaze_DIR to help locating it.")
      else()
        phylanx_info("Warning: Was not able to verify Blaze's presence through CMake. Skipping the error.")
      endif()
    endif()
  endif()

  phylanx_info("Blaze was found.")

endmacro()
