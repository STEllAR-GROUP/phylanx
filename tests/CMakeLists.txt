# Copyright (c) 2017 Harmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs "")

if(PHYLANX_WITH_TESTS_BENCHMARKS)
  set(subdirs ${subdirs} performance)
endif()

if(PHYLANX_WITH_TESTS_REGRESSIONS)
  set(subdirs ${subdirs} regressions)
endif()

if(PHYLANX_WITH_TESTS_UNIT)
  set(subdirs ${subdirs} unit)
endif()

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(tests.${subdir})
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(tests tests.${subdir})
endforeach()

