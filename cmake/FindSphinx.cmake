# Copyright (c) 2018 Bibek Wagle
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

find_program(SPHINX_EXECUTABLE NAMES sphinx-build
    PATHS
    ${SPHINX_ROOT}
    ENV SPHINX_ROOT
    DOC "Sphinx documentation generator"
)

if(SPHINX_EXECUTABLE)
    include(FindPackageHandleStandardArgs)

    find_package_handle_standard_args(Sphinx DEFAULT_MSG
    SPHINX_EXECUTABLE
)
endif()
