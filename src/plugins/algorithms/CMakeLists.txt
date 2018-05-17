# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_phylanx_primitive_plugin(algorithms
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src/plugins/algorithms"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/algorithms"
  AUTOGLOB
  PLUGIN
  FOLDER "Core/Plugins"
  COMPONENT_DEPENDENCIES phylanx)

add_phylanx_pseudo_target(primitives.algorithms_dir.algorithms_plugin)
add_phylanx_pseudo_dependencies(primitives.algorithms_dir
  primitives.algorithms_dir.algorithms_plugin)
add_phylanx_pseudo_dependencies(primitives.algorithms_dir.algorithms_plugin
    algorithms_primitive)
