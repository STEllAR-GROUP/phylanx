# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(example_programs
    kmeans
   )

foreach(example_program ${example_programs})

  set(${example_program}_FLAGS DEPENDENCIES iostreams_component)

  set(sources ${example_program}.cpp)

  source_group("Source Files" FILES ${sources})

  # add example executable
  add_phylanx_executable(${example_program}
                     SOURCES ${sources}
                     ${${example_program}_FLAGS}
                     FOLDER "Examples/Algorithms")

  # add a custom target for this example
  add_phylanx_pseudo_target(examples.algorithms_.${example_program})

  # make pseudo-targets depend on master pseudo-target
  add_phylanx_pseudo_dependencies(examples.algorithms_
                              examples.algorithms_.${example_program})

  # add dependencies to pseudo-target
  add_phylanx_pseudo_dependencies(examples.algorithms_.${example_program}
                              ${example_program}_exe)
endforeach()

