# Copyright (c) 2020 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs
    matrixops
   )

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(tests.regressions.plugins_dir.${subdir}_dir)
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(tests.regressions.plugins_dir
    tests.regressions.plugins_dir.${subdir}_dir)
endforeach()

