# Copyright (c) 2017-2020 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_phylanx_primitive_plugin(matrixops
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src/plugins/matrixops"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops"
  AUTOGLOB
  PLUGIN
  FOLDER "Core/Plugins"
  DEPENDENCIES phylanx_component common)

add_phylanx_pseudo_target(primitives.matrixops_dir.matrixops_plugin)
add_phylanx_pseudo_dependencies(primitives.matrixops_dir
  primitives.matrixops_dir.matrixops_plugin)
add_phylanx_pseudo_dependencies(primitives.matrixops_dir.matrixops_plugin
    matrixops_primitive)
