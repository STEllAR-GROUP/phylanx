# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    simple_als
    simple_lra
   )

set(simple_lra_FLAGS COMPONENT_DEPENDENCIES iostreams)

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/Algorithm/")

  add_phylanx_unit_test("algorithm" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.algorithm.${test})
  add_phylanx_pseudo_dependencies(tests.unit.algorithm tests.unit.algorithm.${test})
  add_phylanx_pseudo_dependencies(tests.unit.algorithm.${test} ${test}_test_exe)

endforeach()


