# Copyright (c) 2018 Bibek Wagle
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

if (DOXYGEN_FOUND)
    # Add files that needs to be passed to doxygen here
    set(DOXYGEN_DEPENDENCIES
        "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/diag_operation.hpp"
        "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/gradient_operation.hpp"
        "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/linearmatrix.hpp"
        "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/linspace.hpp"
        "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/slicing_operation.hpp"
        "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/random.hpp")

    foreach(DOXYGEN_INPUT ${DOXYGEN_DEPENDENCIES})
        set(DOXYGEN_INPUTS  "${DOXYGEN_INPUTS} ${DOXYGEN_INPUT}")
    endforeach()

    set(DOXYGEN_INPUT_FILE ${CMAKE_SOURCE_DIR}/cmake/templates/Doxy.in)
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

    set(DOXYGEN_OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

    configure_file(${DOXYGEN_INPUT_FILE} ${DOXYGEN_OUTPUT_FILE} @ONLY)
    message("Doxygen API Reference will be generated.")

    add_custom_target( doxygendocs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUTPUT_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen."
            VERBATIM )
endif()

if (SPHINX_FOUND)
    file(COPY ${CMAKE_SOURCE_DIR}/docs/sphinx_source DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    set(SPHINX_CONF_INPUT_FILE ${CMAKE_SOURCE_DIR}/cmake/templates/conf.py.in)
    set(SPHINX_CONF_OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/sphinx_source/conf.py)
    configure_file(${SPHINX_CONF_INPUT_FILE} ${SPHINX_CONF_OUTPUT_FILE} @ONLY)

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/sphinx_build"
      COMMAND ${SPHINX_EXECUTABLE} -b html "${CMAKE_CURRENT_BINARY_DIR}/sphinx_source"
      "${CMAKE_CURRENT_BINARY_DIR}/sphinx_build")

    add_custom_target(sphinxdocs ALL
        COMMAND ${SPHINX_EXECUTABLE} -b html "${CMAKE_CURRENT_BINARY_DIR}/sphinx_source"
        "${CMAKE_CURRENT_BINARY_DIR}/sphinx_build"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating Sphinx documentation."
        VERBATIM)
endif()
