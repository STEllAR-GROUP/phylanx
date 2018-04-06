# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs
    arithmetics
   )

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(plugins_.${subdir}_)
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(plugins_ plugins_.${subdir}_)
endforeach()

