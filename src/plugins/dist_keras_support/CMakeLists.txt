# Copyright (c) 2017-2020 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_phylanx_primitive_plugin(dist_keras_support
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src/plugins/dist_keras_support"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/dist_keras_support"
  AUTOGLOB
  PLUGIN
  FOLDER "Core/Plugins"
  COMPONENT_DEPENDENCIES phylanx
  DEPENDENCIES common)

add_phylanx_pseudo_target(primitives.dist_keras_support_dir.dist_keras_support_plugin)
add_phylanx_pseudo_dependencies(primitives.dist_keras_support_dir
    primitives.dist_keras_support_dir.dist_keras_support_plugin)
add_phylanx_pseudo_dependencies(
    primitives.dist_keras_support_dir.dist_keras_support_plugin
    dist_keras_support_primitive)
