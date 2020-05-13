# Copyright (c) 2020 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    1157_astype_bool
    1157_astype_bool_axis
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Regression/Plugins/MatrixOps")

  add_phylanx_regression_test("plugins.matrixops" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.regressions.plugins_dir.matrixops_dir.${test})
  add_phylanx_pseudo_dependencies(tests.regressions.plugins_dir.matrixops_dir
    tests.regressions.plugins_dir.matrixops_dir.${test})
  add_phylanx_pseudo_dependencies(
    tests.regressions.plugins_dir.matrixops_dir.${test}
    ${test}_test_exe)

endforeach()

