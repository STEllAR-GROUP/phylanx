# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    cout_noargs_prints_nothing_159
    cout_prints_twice_155
    define_looses_values_177
    negative_integral_value_131
   )

set(cout_noargs_prints_nothing_FLAGS COMPONENT_DEPENDENCIES iostreams)
set(cout_prints_twice_155_FLAGS COMPONENT_DEPENDENCIES iostreams)
set(define_looses_values_177_FLAGS COMPONENT_DEPENDENCIES iostreams)

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


