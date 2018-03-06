# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    make_list
   )

foreach(test ${tests})
  set(script ${test}.py)

  add_phylanx_python_unit_test("primitives" ${test}
    SCRIPT ${script}
    FOLDER "Tests/Python/Unit/ExecutionTree/Primitives"
    DEPENDS phylanx_py python_setup
    WORKING_DIRECTORY ${PHYLANX_PYTHON_EXTENSION_LOCATION}
    ENVIRONMENT "PYTHONPATH=${PHYLANX_PYTHON_EXTENSION_LOCATION}")

  add_phylanx_pseudo_target(tests.unit.python_execution_tree.python_primitives.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.unit.python_execution_tree.python_primitives
    tests.unit.python_execution_tree.python_primitives.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.unit.python_execution_tree.python_primitives.${test}_py
    ${test}_test_py)

endforeach()

