# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# add inpect executable

file(GLOB INSPECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB INSPECT_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} *.hpp)
add_executable(inspect EXCLUDE_FROM_ALL ${INSPECT_SOURCES} ${INSPECT_HEADERS})
set_property(TARGET inspect PROPERTY FOLDER "Tools\\Inspect")

target_compile_definitions(inspect PRIVATE
  "PHYLANX_HAVE_GIT_COMMIT=\"${PHYLANX_WITH_GIT_COMMIT}\"")

target_link_libraries(inspect PRIVATE hpx)

# add dependencies to pseudo-target
add_hpx_pseudo_dependencies(tools.inspect inspect)
