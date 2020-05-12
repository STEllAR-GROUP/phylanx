# Copyright (c) 2017-2020 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs
    ast
    distributed
    execution_tree
    plugins
    python
   )

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(tests.regressions.${subdir}_dir)
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(tests.regressions tests.regressions.${subdir}_dir)
endforeach()

