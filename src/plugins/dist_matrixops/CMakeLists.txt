# Copyright (c) 2017-2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_phylanx_primitive_plugin(dist_matrixops
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src/plugins/dist_matrixops"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/dist_matrixops"
  AUTOGLOB
  PLUGIN
  FOLDER "Core/Plugins"
  DEPENDENCIES phylanx_component common)

add_phylanx_pseudo_target(primitives.dist_matrixops_dir.dist_matrixops_plugin)
add_phylanx_pseudo_dependencies(primitives.dist_matrixops_dir
    primitives.dist_matrixops_dir.dist_matrixops_plugin)
add_phylanx_pseudo_dependencies(
    primitives.dist_matrixops_dir.dist_matrixops_plugin
    dist_matrixops_primitive)
