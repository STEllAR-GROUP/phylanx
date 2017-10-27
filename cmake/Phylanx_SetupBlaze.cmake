# Copyright (c) 2017 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup Blaze as a dependency


macro(phylanx_setup_blaze)
  find_package(blaze NO_CMAKE_PACKAGE_REGISTRY)
  if(NOT blaze_FOUND)
    phylanx_error("Blaze could not be found. Please specify BLAZE_DIR to help locating it.")
  endif()
  add_library(blaze_target INTERFACE)
  target_link_libraries(blaze_target INTERFACE blaze::blaze)

  
endmacro()
