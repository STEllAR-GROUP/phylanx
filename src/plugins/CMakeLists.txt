# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs
    algorithms
    arithmetics
    booleans
    controls
    fileio
    listops
    matrixops
    solvers
   )

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(primitives.${subdir}_dir)
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(primitives primitives.${subdir}_dir)
endforeach()

set(phylanx_targets ${phylanx_targets} PARENT_SCOPE)
