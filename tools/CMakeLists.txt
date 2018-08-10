# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
cmake_minimum_required(VERSION 3.9)

set(subdirs inspect)

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(tools.${subdir})
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(tools tools.${subdir})
endforeach()

