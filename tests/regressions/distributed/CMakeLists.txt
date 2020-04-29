# Copyright (c) 2017-2020 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    1155_wrong_annotation_name
    1155_wrong_broadcast
   )

foreach(test ${tests})
  set(${test}_PARAMETERS LOCALITIES 2)

  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Regression/Distributed/")

  add_phylanx_regression_test("distributed" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.regressions.distributed_dir.${test})
  add_phylanx_pseudo_dependencies(tests.regressions.distributed_dir
    tests.regressions.distributed_dir.${test})
  add_phylanx_pseudo_dependencies(tests.regressions.distributed_dir.${test}
    ${test}_test_exe)

endforeach()


