# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# add inpect executable

add_phylanx_executable(inspect
  AUTOGLOB
  NOLIBS
  EXCLUDE_FROM_ALL
  FOLDER "Tools/Inspect")

set_property(TARGET inspect_exe APPEND
  PROPERTY COMPILE_DEFINITIONS
  "PHYLANX_HAVE_GIT_COMMIT=\"${PHYLANX_WITH_GIT_COMMIT}\"")

if((NOT MSVC) OR HPX_WITH_VCPKG)
  target_link_libraries(inspect_exe ${HPX_LIBRARIES})
endif()

# add dependencies to pseudo-target
add_hpx_pseudo_dependencies(tools.inspect inspect_exe)

