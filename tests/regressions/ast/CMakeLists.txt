# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    list_matching_178
    trailing_comments_413
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Regression/Ast/")

  add_phylanx_regression_test("ast" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.regressions.ast_dir.${test})
  add_phylanx_pseudo_dependencies(tests.regressions.ast_dir
    tests.regressions.ast_dir.${test})
  add_phylanx_pseudo_dependencies(tests.regressions.ast_dir.${test}
    ${test}_test_exe)

endforeach()


