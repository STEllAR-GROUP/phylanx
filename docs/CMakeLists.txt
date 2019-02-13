# Copyright (c) 2012 Bryce Adelstein-Lelbach
# Copyright (c) 2018 Bibek Wagle
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Add all files here which should be passed to doxygen
set(doxygen_dependencies
    "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/diag_operation.hpp"
    "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/gradient_operation.hpp"
    "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/linearmatrix.hpp"
    "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/linspace.hpp"
    "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/slicing_operation.hpp"
    "${PROJECT_SOURCE_DIR}/phylanx/plugins/matrixops/random.hpp")

foreach(doxygen_input ${doxygen_dependencies})
  set(doxygen_inputs "${doxygen_inputs} ${doxygen_input}")
endforeach()

set(doxygen_output_file "${CMAKE_CURRENT_BINARY_DIR}/phylanx_autodoc")
set(doxygen_output_dir "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

set(doxygen_definition_list
    "DOXYGEN:=1"
    "BOOST_SYSTEM_NOEXCEPT="
    "PHYLANX_EXCEPTION_EXPORT="
    "PHYLANX_EXPORT="
    "PHYLANX_API_EXPORT="
    "PHYLANX_ALWAYS_EXPORT="
    "extern="
    "HPX_FORCEINLINE=")

# Definitions for autodoc.doxy.in
set(doxygen_output_file "${CMAKE_CURRENT_BINARY_DIR}/phylanx_autodoc")
set(doxygen_output_dir "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

foreach(doxygen_predef ${doxygen_definition_list})
  set(doxygen_definitions "${doxygen_definitions} \"${doxygen_predef}\"")
endforeach()

foreach(doxygen_input ${doxygen_dependencies})
  set(doxygen_inputs "${doxygen_inputs} ${doxygen_input}")
endforeach()

phylanx_info("Creating Doxyfile: ${CMAKE_CURRENT_BINARY_DIR}/phylanx_autodoc.doxy")
configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/autodoc.doxy.in"
                "${CMAKE_CURRENT_BINARY_DIR}/phylanx_autodoc.doxy"
                @ONLY)


# Configure Sphinx conf.py
if(PHYLANX_WITH_GIT_TAG)
  set(PHYLANX_CONF_DOCUMENTATION_VERSION "${PHYLANX_WITH_GIT_TAG}")
elseif(PHYLANX_WITH_GIT_BRANCH)
  set(PHYLANX_CONF_DOCUMENTATION_VERSION "${PHYLANX_WITH_GIT_BRANCH}")
else()
  # PHYLANX_VERSION is always available, use as fallback
  set(PHYLANX_CONF_DOCUMENTATION_VERSION "${PHYLANX_VERSION}")
endif()

set(SPHINX_CONF_PY_DEST
    "${CMAKE_CURRENT_BINARY_DIR}/sphinx/conf.py")
configure_file(
  "${PROJECT_SOURCE_DIR}/cmake/templates/conf.py.in"
  ${SPHINX_CONF_PY_DEST}
  @ONLY
)

# Copy the Sphinx source files to build directory
file(GLOB_RECURSE sphinx_source_files LIST_DIRECTORIES false RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}/" "sphinx/*")

foreach(sphinx_source_file ${sphinx_source_files})
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${sphinx_source_file}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${sphinx_source_file}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/${sphinx_source_file}"
      "${CMAKE_CURRENT_BINARY_DIR}/${sphinx_source_file}")
endforeach()

foreach(sphinx_source_file ${sphinx_source_files})
  set(sphinx_source_files_build ${sphinx_source_files_build} "${CMAKE_CURRENT_BINARY_DIR}/${sphinx_source_file}")
endforeach()

# Create links to source files so that they are visible to sphinx
create_symbolic_link("${PROJECT_SOURCE_DIR}/src"
                     "${CMAKE_CURRENT_BINARY_DIR}/src")
create_symbolic_link("${PROJECT_SOURCE_DIR}/phylanx"
                     "${CMAKE_CURRENT_BINARY_DIR}/phylanx")
create_symbolic_link("${PROJECT_SOURCE_DIR}/examples"
                     "${CMAKE_CURRENT_BINARY_DIR}/examples")
create_symbolic_link("${PROJECT_SOURCE_DIR}/tests"
                     "${CMAKE_CURRENT_BINARY_DIR}/tests")

phylanx_source_to_doxygen(phylanx_autodoc
  DEPENDENCIES ${doxygen_dependencies})

set(SPHINX_DOCS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/share/phylanx/docs/html")

add_custom_command(
  OUTPUT "${SPHINX_DOCS_OUTPUT_DIR}"
  DEPENDS "${sphinx_source_files_build}"
    "${CMAKE_CURRENT_BINARY_DIR}/phylanx_autodoc/index.xml"
  COMMAND ${SPHINX_EXECUTABLE} -b html -n -d
    "${CMAKE_CURRENT_BINARY_DIR}/doctree"
    "${CMAKE_CURRENT_BINARY_DIR}/sphinx"
    "${SPHINX_DOCS_OUTPUT_DIR}")

add_custom_target(docs
  ALL
  DEPENDS ${SPHINX_DOCS_OUTPUT_DIR})

add_custom_target(git_docs
  COMMAND "${CMAKE_COMMAND}"
    -DPHYLANX_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
    -DPHYLANX_BINARY_DIR:PATH=${CMAKE_BINARY_DIR}
    -DPHYLANX_VERSION:STRING=${PHYLANX_VERSION}
    -DPHYLANX_WITH_GIT_BRANCH=${PHYLANX_WITH_GIT_BRANCH}
    -DPHYLANX_WITH_GIT_TAG=${PHYLANX_WITH_GIT_TAG}
    -P "${CMAKE_SOURCE_DIR}/cmake/Phylanx_UpdateGitDocs.cmake")

set_target_properties(git_docs
    PROPERTIES FOLDER "Documentation/")

add_dependencies(git_docs docs)
