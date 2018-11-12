# Copyright (c) 2018 Patrick Diehl
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

find_package(PkgConfig QUIET)

find_path(BLAZEITERATIVE_INCLUDE BlazeIterative/BlazeIterative.hpp
  HINTS /usr/include /usr/local/include "${BLAZEITERATIVE_DIR}/include/" )

mark_as_advanced(BLAZEITERATIVE_DIR)
mark_as_advanced(BLAZEITERATIVE_INCLUDE)

if(NOT BLAZEITERATIVE_INCLUDE)
  phylanx_error("Blazeiterative could not be found. Please specify BLAZEITERATIVE_DIR to assist locating it")
else()
  include_directories(${BLAZEITERATIVE_INCLUDE}/BlazeIterative)
  phylanx_add_config_define(PHYLANX_HAVE_BLAZEITERATIVE)
endif()
