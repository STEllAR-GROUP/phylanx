# Copyright (c) 2018 Alireza Kheirkhahan
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup pybind11 as a dependency
if(PHYLANX_WITH_HIGHFIVE)

    find_package(hdf5 NO_MODULE NO_CMAKE_PACKAGE_REGISTRY)
    if(NOT HDF5_FOUND)
        message("The HDF5 libraries could not be found, please set HDF5_DIR to help locating it.")
    endif()

    find_package(HighFive NO_MODULE NO_CMAKE_PACKAGE_REGISTRY)
    if(NOT HighFive_FOUND)
        message("HighFive could not be found, please set HighFive_DIR to help locating it.")
    endif()

endif(PHYLANX_WITH_HIGHFIVE)
