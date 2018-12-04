# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    eval
    dictionary
    for
    slice
    make_array
    map_numpy
    set_operation
    parallel
    multi_return
   )

foreach(test ${tests})
  set(script ${test}.py)

  add_phylanx_python_unit_test("execution_tree" ${test}
    SCRIPT ${script}
    FOLDER "Tests/Python/Unit/ExecutionTree"
    DEPENDS phylanx_py python_setup
    WORKING_DIRECTORY ${PHYLANX_PYTHON_EXTENSION_LOCATION}
    ENVIRONMENT "PYTHONPATH=${PHYLANX_PYTHON_EXTENSION_LOCATION}")

  add_phylanx_pseudo_target(tests.unit.python_execution_tree.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.unit.python_execution_tree tests.unit.python_execution_tree.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.unit.python_execution_tree.${test}_py ${test}_test_py)

endforeach()

set(subdirs
    primitives
   )

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(tests.unit.python_execution_tree.python_${subdir})
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(
    tests.unit.python_execution_tree
    tests.unit.python_execution_tree.python_${subdir})
endforeach()

