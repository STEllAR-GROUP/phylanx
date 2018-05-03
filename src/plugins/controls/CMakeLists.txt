# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_phylanx_primitive_plugin(controls
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src/plugins/controls"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/controls"
  AUTOGLOB
  PLUGIN
  FOLDER "Core/Plugins"
  COMPONENT_DEPENDENCIES phylanx)

add_phylanx_pseudo_target(primitives.controls_dir.controls_plugin)
add_phylanx_pseudo_dependencies(primitives.controls_dir
  primitives.controls_dir.controls_plugin)
add_phylanx_pseudo_dependencies(primitives.controls_dir.controls_plugin
    controls_primitive)
