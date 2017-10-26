# Copyright (c) 2017 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup Blaze as a dependency


macro(phylanx_setup_blaze)
  # Step 1: See Blaze is made available through `blaze_INCLUDE_DIR`
  if(blaze_INCLUDE_DIR)
    phylanx_info("Adding defined blaze_INCLUDE_DIR to header include directories.")
    list(APPEND CMAKE_REQUIRED_INCLUDES ${blaze_INCLUDE_DIR})
    include_directories(${blaze_INCLUDE_DIR})
  endif()
  # Step 2: See if Vcpkg is being used
  if(_VCPKG_ROOT_DIR)
    phylanx_info("Vcpkg found. Adding to header include directory: " ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)
    list(APPEND CMAKE_REQUIRED_INCLUDES "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
    include_directories("${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
  endif()

  # Step 3: Check if Math.h is available
  include(CheckIncludeFileCXX)
  # Ensure CMake tells the compiler to use C++14 or the Blaze check command will fail
  if(MSCVC)
    set(CMAKE_REQUIRED_FLAGS "/std:c++14")
  else()
    set(CMAKE_REQUIRED_FLAGS "-std=c++14")
  endif()
  check_include_file_cxx(blaze/Math.h HAVE_BLAZE_MATH_H)
  if(HAVE_BLAZE_MATH_H)
    phylanx_info("Blaze was found.")
  else()
    # Step 4: See if Blaze is available through find_package
    find_package(blaze NO_CMAKE_PACKAGE_REGISTRY)
    if(NOT blaze_FOUND)
      phylanx_error("Blaze could not be found. Please set one of the following flags to help locating it. \n"
                    "    Specify blaze_DIR if Blaze is available through CMake\n"
                    "    Specify blaze_INCLUDE_DIR if only Blaze headers are available")
    endif()
  endif()
  add_library(blaze_target INTERFACE)
  target_link_libraries(blaze_target INTERFACE blaze::blaze)

  
endmacro()
