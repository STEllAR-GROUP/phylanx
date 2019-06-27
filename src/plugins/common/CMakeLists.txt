# Copyright (c) 2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_phylanx_library(common
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src/plugins/common"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/common"
  AUTOGLOB
  FOLDER "Core/Plugins"
  COMPONENT_DEPENDENCIES phylanx)

target_compile_definitions(common PRIVATE PHYLANX_COMMON_EXPORTS)

add_phylanx_pseudo_target(primitives.common_dir.common)
add_phylanx_pseudo_dependencies(primitives.common_dir primitives.common_dir.common)
add_phylanx_pseudo_dependencies(primitives.common_dir.common common)
