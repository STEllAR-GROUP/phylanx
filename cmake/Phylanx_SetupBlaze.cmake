# Copyright (c) 2017 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup Blaze as a dependency

# TODO: We don't use `phylanx_setup_eigen3`. We can remove `Phylanx_SetupEigen3.cmake`.

macro(phylanx_setup_blaze)

  if(blaze_INCLUDE_DIR)
	list(APPEND CMAKE_REQUIRED_INCLUDES ${blaze_INCLUDE_DIR})
	include_directories(${blaze_INCLUDE_DIR})
  elseif(blaze_DIR)
	find_package(blaze)
  endif()
  
  include(CheckIncludeFileCXX)
	check_include_file_cxx(blaze/Math.h HAVE_BLAZE_MATH_H)
  # HACK: PHYLANX_SKIP_BLAZE_CHECK - If Blaze is available through vcpkg, vcpkg's toolchain script does not offer any variables to detect blaze's location when this script is running.
  if(NOT HAVE_BLAZE_MATH_H)
    if(NOT PHYLANX_SKIP_BLAZE_CHECK)
	  phylanx_error("Blaze could not be found. Please set blaze_DIR to help locating it.")
	else()
	  phylanx_info("Skipping Blaze presence check.")
	endif()
  endif()


  phylanx_info("Blaze was found.")

endmacro()
