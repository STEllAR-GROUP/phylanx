# Copyright (c) 2018 Parsa Amini
# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    remote_run
    remote_arrays_addition
   )

foreach(test ${tests})
  set(${test}_FLAGS DEPENDENCIES iostreams_component)
  set(${test}_PARAMETERS LOCALITIES 2)

  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Distributed/")

  add_phylanx_unit_test("distributed" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.distributed.${test})
  add_phylanx_pseudo_dependencies(tests.unit.distributed tests.unit.distributed.${test})
  add_phylanx_pseudo_dependencies(tests.unit.distributed.${test} ${test}_test_exe)
endforeach()

