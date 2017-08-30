# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs
    ast
    config
    ir
    util
   )

foreach(subdir ${subdirs})
  add_hpx_pseudo_target(tests.unit.${subdir})
  add_subdirectory(${subdir})
  add_hpx_pseudo_dependencies(tests.unit tests.unit.${subdir})
endforeach()

