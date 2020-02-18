# Copyright (c) 2017-2018 Hartmut Kaiser
# Copyright (c) 2018 Shahrzad Shirzad
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_phylanx_primitive_plugin(solvers
    SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src/plugins/solvers"
    HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/solvers"
    AUTOGLOB
    PLUGIN FOLDER "Core/Plugins"
    DEPENDENCIES phylanx_component
)

add_phylanx_pseudo_target(primitives.solvers_dir.solvers_plugin)
add_phylanx_pseudo_dependencies(
    primitives.solvers_dir primitives.solvers_dir.solvers_plugin)
add_phylanx_pseudo_dependencies(
    primitives.solvers_dir.solvers_plugin solvers_primitive)
