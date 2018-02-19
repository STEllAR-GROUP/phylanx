# Copyright (c) 2018 Alireza Kheirkhahan
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup HighFive/HDF5 as a dependency
macro(phylanx_setup_hdf5)
    if(PHYLANX_WITH_HIGHFIVE)

        find_package(HDF5 NO_MODULE NO_CMAKE_PACKAGE_REGISTRY)
        if(NOT HDF5_FOUND)
            phylanx_warn("The HDF5 libraries could not be found, please set HDF5_DIR to help locating it.")
            set(PHYLANX_WITH_HIGHFIVE OFF)
        else()
            phylanx_info("HDF5 library version: " ${HDF5_VERSION})

            find_package(HighFive NO_MODULE NO_CMAKE_PACKAGE_REGISTRY)
            if(NOT HighFive_FOUND)
                phylanx_warn("HighFive could not be found, please set HighFive_DIR to help locating it.")
                set(PHYLANX_WITH_HIGHFIVE OFF)
            else()
                phylanx_info("Found HighFive")
            endif()
        endif()
    endif()
endmacro()
