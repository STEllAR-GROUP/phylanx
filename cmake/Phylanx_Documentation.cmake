# Copyright (c) 2018 Bibek Wagle
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

if (PHYLANX_WITH_DOCUMENTATION)
    # check if Doxygen and Sphinx are installed
    find_package(Doxygen)
    find_package(Sphinx)

    if (NOT DOXYGEN_FOUND)
        message("Doxygen not found. Doxygen API Reference  can not be generated.")
    endif ()

    if (NOT SPHINX_FOUND)
        message("Sphinx not found. Sphinx Documentation  can not be generated.")
    endif ()
endif(PHYLANX_WITH_DOCUMENTATION)
