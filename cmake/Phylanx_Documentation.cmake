# Copyright (c) 2018 Bibek Wagle
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

if (PHYLANX_WITH_DOCUMENTATION)
    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        #Add files that needs to be passed to doxygen here
        set(DOXYGEN_DEPENDENCIES
                "${PROJECT_SOURCE_DIR}/phylanx/execution_tree/primitives/diag_operation.hpp"
                "${PROJECT_SOURCE_DIR}/phylanx/execution_tree/primitives/linearmatrix.hpp"
                "${PROJECT_SOURCE_DIR}/phylanx/execution_tree/primitives/linspace.hpp"
                "${PROJECT_SOURCE_DIR}/phylanx/execution_tree/primitives/row_slicing.hpp"
                "${PROJECT_SOURCE_DIR}/phylanx/execution_tree/primitives/column_slicing.hpp"
                "${PROJECT_SOURCE_DIR}/phylanx/execution_tree/primitives/slicing_operation.hpp"
                "${PROJECT_SOURCE_DIR}/phylanx/execution_tree/primitives/set_operation.hpp"
                "${PROJECT_SOURCE_DIR}/phylanx/execution_tree/primitives/column_set.hpp"
                "${PROJECT_SOURCE_DIR}/phylanx/execution_tree/primitives/row_set.hpp")


        foreach(DOXYGEN_INPUT ${DOXYGEN_DEPENDENCIES})
            set(DOXYGEN_INPUTS  "${DOXYGEN_INPUTS} ${DOXYGEN_INPUT}")
        endforeach()

        set(DOXYGEN_INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/Doxy.in)
        set(DOXYGEN_OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_INPUT_FILE} ${DOXYGEN_OUTPUT_FILE} @ONLY)
        message("Documentation will be built")

        add_custom_target( htmldocs ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUTPUT_FILE}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM )
    else (DOXYGEN_FOUND)
        message("Doxygen not found. Docs can not be built")
    endif (DOXYGEN_FOUND)
endif(PHYLANX_WITH_DOCUMENTATION)
