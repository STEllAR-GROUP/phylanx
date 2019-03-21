# Copyright (c) 2017-2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup pybind11 as a dependency
macro(phylanx_setup_pybind11)

  if(MSVC AND PHYLANX_WITH_CXX17)
    set(PYBIND11_CPP_STANDARD /std:c++17)
  endif()

  phylanx_info("Used C++ flags for pybind11: " ${PYBIND11_CPP_STANDARD})

  find_package(pybind11 REQUIRED NO_CMAKE_PACKAGE_REGISTRY)
  if(NOT pybind11_FOUND)
    phylanx_error("pybind11 could not be found, please set pybind11_DIR to help locating it.")
  endif()

  if(${pybind11_VERSION} VERSION_LESS 2.2.0)
    phylanx_error("pybind11 too old, should be of version 2.2.0 or newer.")
  endif()

  # Make sure the Python interpreter found is sufficiently new
  if(${PYTHON_VERSION_STRING} VERSION_LESS 3.6)
    phylanx_error("The python version found is too old (${PYTHON_VERSION_STRING}), Phylanx requires at least Python V3.6")
  endif()

  phylanx_info("Python library version: " ${PYTHON_VERSION_STRING})
  phylanx_info("Python executable: " ${PYTHON_EXECUTABLE})
  phylanx_info("Python libraries: " ${PYTHON_LIBRARIES})
  phylanx_info("Pybind11 library version: " ${pybind11_VERSION})

endmacro()
