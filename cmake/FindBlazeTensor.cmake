# Copyright (c) 2018 Patrick Diehl
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

find_package(PkgConfig QUIET)

find_path(BLAZETENSOR_INCLUDE blaze_tensor/Blaze.h
  HINTS /usr/include /usr/local/include "${blaze_tensor_DIR}" )

mark_as_advanced(blaze_tensor_DIR)
mark_as_advanced(BLAZETENSOR_INCLUDE)

if(NOT BLAZETENSOR_INCLUDE)
  phylanx_error("BlazeTensor could not be found. Please specify blaze_tensor_DIR to assist locating it")
else()
  include_directories(${BLAZETENSOR_INCLUDE})
  phylanx_add_config_define(PHYLANX_HAVE_BLAZETENSOR)
endif()
