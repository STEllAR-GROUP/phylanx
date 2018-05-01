# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs
    arithmetics
    booleans
    controls
    fileio
    listops
    matrixops
   )

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(primitives.${subdir}_)
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(primitives primitives.${subdir}_)
endforeach()

set(phylanx_targets ${phylanx_targets} PARENT_SCOPE)
