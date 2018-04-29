# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs
    ast
    execution_tree
    util
   )

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(tests.unit.python_${subdir})
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(tests.unit.python tests.unit.python_${subdir})
endforeach()

