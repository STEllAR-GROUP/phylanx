# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    negative_integral_value_131
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Regression/ExecutionTree/")

  add_phylanx_regression_test("execution_tree" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.regressions.execution_tree_.${test})
  add_phylanx_pseudo_dependencies(tests.regressions.execution_tree_
    tests.regressions.execution_tree_.${test})
  add_phylanx_pseudo_dependencies(tests.regressions.execution_tree_.${test} 
    ${test}_test_exe)

endforeach()


