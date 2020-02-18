# Copyright (c) 2017-2020 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_phylanx_primitive_plugin(statistics
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src/plugins/statistics"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/statistics"
  AUTOGLOB
  PLUGIN
  FOLDER "Core/Plugins"
  DEPENDENCIES phylanx_component
  DEPENDENCIES common)

add_phylanx_pseudo_target(primitives.statistics_dir.statistics_plugin)
add_phylanx_pseudo_dependencies(primitives.statistics_dir
  primitives.statistics_dir.statistics_plugin)
add_phylanx_pseudo_dependencies(primitives.statistics_dir.statistics_plugin
    statistics_primitive)
