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
    DEPENDS python)

  add_phylanx_pseudo_target(tests.unit.python.ast.${test}_py)
  add_phylanx_pseudo_dependencies(tests.unit.python.ast tests.unit.python.ast.${test}_py)
  add_phylanx_pseudo_dependencies(tests.unit.python.ast.${test}_py ${test}_test_py)

endforeach()

