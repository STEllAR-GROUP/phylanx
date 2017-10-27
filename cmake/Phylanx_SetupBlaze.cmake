# Copyright (c) 2017 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup Blaze as a dependency


macro(phylanx_setup_blaze)
  find_package(blaze NO_CMAKE_PACKAGE_REGISTRY)
  if(NOT blaze_FOUND)
    phylanx_error("Blaze could not be found. Please set one of the following flags to help locating it. \n"
                  "    Specify blaze_DIR if Blaze is available through CMake\n"
                  "    Specify blaze_INCLUDE_DIR if only Blaze headers are available")
  endif()
  add_library(blaze_target INTERFACE)
  target_link_libraries(blaze_target INTERFACE blaze::blaze)

  
endmacro()
