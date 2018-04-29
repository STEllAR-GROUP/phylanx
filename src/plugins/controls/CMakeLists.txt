# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_phylanx_primitive_plugin(phylanx_controls
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}/src/plugins/controls"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/controls"
  AUTOGLOB
  FOLDER "Core/Plugins"
  COMPONENT_DEPENDENCIES phylanx)

