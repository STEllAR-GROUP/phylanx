# Copyright (c) 2017-2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    annotation
    annotation_2_loc
    compiler
    compiler_component
    expression_topology
    function_call_arguments
    generate_tree
    parse_primitive_name
    variable_definition
   )

set(annotation_2_loc_PARAMETERS LOCALITIES 2)

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/ExecutionTree/")

  add_phylanx_unit_test("execution_tree" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.execution_tree.${test})
  add_phylanx_pseudo_dependencies(tests.unit.execution_tree
    tests.unit.execution_tree.${test})
  add_phylanx_pseudo_dependencies(tests.unit.execution_tree.${test} ${test}_test_exe)

endforeach()


set(subdirs
    primitives)

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(tests.unit.execution_tree.${subdir}_)
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(tests.unit.execution_tree
    tests.unit.execution_tree.${subdir}_)
endforeach()

