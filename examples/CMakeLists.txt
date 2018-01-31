# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs
    algorithms
    ast
	profiling
   )

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(examples.${subdir}_)
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(examples examples.${subdir}_)
endforeach()

