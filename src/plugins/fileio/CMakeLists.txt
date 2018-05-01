# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(headers
   "${PROJECT_SOURCE_DIR}/phylanx/plugins/fileio/fileio.hpp"
   "${PROJECT_SOURCE_DIR}/phylanx/plugins/fileio/file_read.hpp"
   "${PROJECT_SOURCE_DIR}/phylanx/plugins/fileio/file_read_csv.hpp"
   "${PROJECT_SOURCE_DIR}/phylanx/plugins/fileio/file_write.hpp"
   "${PROJECT_SOURCE_DIR}/phylanx/plugins/fileio/file_write_csv.hpp"
  )
set(sources
   "fileio.cpp"
   "file_read.cpp"
   "file_read_csv.cpp"
   "file_write.cpp"
   "file_write_csv.cpp"
  )

if(PHYLANX_WITH_HIGHFIVE)
  set(headers ${headers}
     "${PROJECT_SOURCE_DIR}/phylanx/plugins/fileio/file_read_hdf5.hpp"
     "${PROJECT_SOURCE_DIR}/phylanx/plugins/fileio/file_write_hdf5.hpp"
    )
  set(sources ${sources} "file_read_hdf5.cpp" "file_write_hdf5.cpp")
endif()

add_phylanx_primitive_plugin(fileio
  SOURCES ${sources}
  HEADERS ${headers}
  HEADER_ROOT "${PROJECT_SOURCE_DIR}/phylanx/plugins/fileio"
  PLUGIN
  FOLDER "Core/Plugins"
  COMPONENT_DEPENDENCIES phylanx)

if(PHYLANX_WITH_HIGHFIVE)
  target_link_libraries(fileio_primitive
    PRIVATE
      HighFive
      ${PHYLANX_HDF5_LIBRARIES})
endif()

add_phylanx_pseudo_target(primitives.fileio_.fileio_plugin)
add_phylanx_pseudo_dependencies(primitives.fileio_
  primitives.fileio_.fileio_plugin)
add_phylanx_pseudo_dependencies(primitives.fileio_.fileio_plugin
    fileio_primitive)
