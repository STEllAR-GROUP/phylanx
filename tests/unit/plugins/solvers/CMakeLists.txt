# Copyright (c) 2018 Shahrzad Shirzad
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
        linear_solver
        )

foreach(test ${tests})
    set(sources ${test}.cpp)

    source_group("Source Files" FILES ${sources})

    # add executable
    add_phylanx_executable(${test}_test
            SOURCES ${sources}
            ${${test}_FLAGS}
            EXCLUDE_FROM_ALL
            FOLDER "Tests/Unit/Plugins/Solvers")

    add_phylanx_unit_test("plugins.solvers" ${test} ${${test}_PARAMETERS})

    add_phylanx_pseudo_target(tests.unit.plugins.solvers.${test})
    add_phylanx_pseudo_dependencies(tests.unit.plugins.solvers
            tests.unit.plugins.solvers.${test})
    add_phylanx_pseudo_dependencies(tests.unit.plugins.solvers.${test}
            ${test}_test_exe)

endforeach()
