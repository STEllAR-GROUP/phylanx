# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    generate_ast
    node
    traverse_ast
   )

foreach(test ${tests})
  set(script ${test}.py)

  add_phylanx_python_unit_test("ast" ${test}
    SCRIPT ${script}
    FOLDER "Tests/Python/Unit/AST"
    DEPENDS phylanx_py python_setup
    WORKING_DIRECTORY ${PHYLANX_PYTHON_EXTENSION_LOCATION}
    ENVIRONMENT "PYTHONPATH=${PHYLANX_PYTHON_EXTENSION_LOCATION}")

  add_phylanx_pseudo_target(tests.unit.python_ast.${test}_py)
  add_phylanx_pseudo_dependencies(tests.unit.python_ast tests.unit.python_ast.${test}_py)
  add_phylanx_pseudo_dependencies(tests.unit.python_ast.${test}_py ${test}_test_py)

endforeach()

