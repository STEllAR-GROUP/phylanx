# Copyright (c) 2018 Alireza Kheirkhahan
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup HighFive/HDF5 as a dependency
macro(phylanx_setup_highfive)

    set(PHYLANX_HDF5_LIBRARIES)
    if(PHYLANX_WITH_HIGHFIVE)

        # try to find cmake built HDF5 libraries
        find_package(HDF5 NAMES hdf5 COMPONENTS C shared NO_MODULE QUIET)
        if(NOT HDF5_FOUND)
            find_package(HDF5) # NAMES hdf5 COMPONENTS C static NO_MODULE QUIET)
            if(NOT HDF5_FOUND)
                phylanx_warn("The HDF5 libraries could not be found, please set HDF5_DIR to help locating it.")
                set(PHYLANX_WITH_HIGHFIVE OFF)
            endif()
        endif()

        if(HDF5_FOUND)
            phylanx_add_config_define(PHYLANX_HAVE_HDF5)
            include_directories(${HDF5_INCLUDE_DIR})
            foreach(comp ${HDF5_VALID_COMPONENTS})
                string(TOUPPER ${comp} comp_uc)
                if(HDF5_C_${comp_uc}_LIBRARY)
                    link_libraries(${HDF5_C_${comp_uc}_LIBRARY})
                    set(PHYLANX_HDF5_LIBRARIES "${PHYLANX_HDF5_LIBRARIES}" "${HDF5_C_${comp_uc}_LIBRARY}")
                endif()
            endforeach()
            phylanx_info("HDF5 library version: " ${HDF5_VERSION})

            find_package(HighFive NO_MODULE NO_CMAKE_PACKAGE_REGISTRY)
            if(NOT HighFive_FOUND)
                phylanx_warn("HighFive could not be found, please set HighFive_DIR to help locating it.")
                set(PHYLANX_WITH_HIGHFIVE OFF)
            else()
                phylanx_add_config_define(PHYLANX_HAVE_HIGHFIVE)
                phylanx_info("Found HighFive")
            endif()
        endif()
    endif()
endmacro()
