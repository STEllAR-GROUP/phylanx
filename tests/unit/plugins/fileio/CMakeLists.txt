# Copyright (c) 2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    dist_read_csv_2_loc
    file_primitives
    file_csv_primitives
   )

set(dist_read_csv_2_loc_PARAMETERS LOCALITIES 2)

if(PHYLANX_WITH_HIGHFIVE)
  set(tests ${tests}
        file_hdf5_primitives
     )
endif()

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/FileIO")

  add_phylanx_unit_test("plugins.fileio" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.plugins.fileio.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.fileio
    tests.unit.plugins.fileio.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.fileio.${test} ${test}_test_exe)

endforeach()

