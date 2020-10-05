# Copyright (c) 2019 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    argsort
    linearalg
   )

foreach(test ${tests})
  set(script ${test}.py)

  add_phylanx_python_unit_test("matrixops" ${test}
    SCRIPT ${script}
    FOLDER "Tests/Python/Unit/Plugins/Matrixops"
    DEPENDS phylanx_py python_setup
    WORKING_DIRECTORY ${PHYLANX_PYTHON_EXTENSION_LOCATION}
    ENVIRONMENT "PYTHONPATH=${PHYLANX_PYTHON_EXTENSION_LOCATION}:$ENV{PYTHONPATH}")

  add_phylanx_pseudo_target(tests.unit.python_plugins.python_matrixops.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.unit.python_plugins.python_matrixops
    tests.unit.python_plugins.python_matrixops.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.unit.python_plugins.python_matrixops.${test}_py
    ${test}_test_py)

endforeach()

