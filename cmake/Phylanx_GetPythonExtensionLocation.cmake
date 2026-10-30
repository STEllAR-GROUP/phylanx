# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

macro(phylanx_get_python_extension_location suffix result)

  if(${suffix} STREQUAL ".so")
    message(SEND_ERROR "No .so file found in Python libraray, \
      Phylanx does not support static libraries, \
      Phylanx found ${PYTHON_LIBRARIES} ")

  else()
    # setup.py build stages the package under the directory chosen by
    # setuptools' build command (lib.<platform>-<cache tag>). Parsing the
    # extension suffix only works for a two-digit tag such as cpython-35.
    set(_phylanx_platlib "")
    if(PYTHON_EXECUTABLE)
      execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -c
          "import os, sys\ntry:\n    from setuptools import Distribution\n    from setuptools.command.build import build\nexcept ImportError:\n    from distutils.core import Distribution\n    from distutils.command.build import build\ndist = Distribution()\ncmd = build(dist)\ncmd.finalize_options()\nsys.stdout.write(os.path.basename(cmd.build_platlib))\n"
        OUTPUT_VARIABLE _phylanx_platlib
        ERROR_VARIABLE _phylanx_platlib_err
        RESULT_VARIABLE _phylanx_platlib_rc
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE)
      if(NOT _phylanx_platlib_rc EQUAL 0)
        set(_phylanx_platlib "")
      endif()
    endif()

    if(_phylanx_platlib STREQUAL "")
      # extract parts of suffix and generate needed output structure
      if(MSVC)
        # suffix will be something like '.cp36-win_amd64.pyd'
        # result should be something like 'lib.win-amd64-3.6'
        string(REGEX REPLACE
              "\\.cp([0-9])([0-9])-(.*)_(.*)\\..*"
              "lib.\\3-\\4-\\1.\\2"
          _phylanx_platlib ${suffix})
      else()
        # Pybind gets the architecture wrong for POWER8.
        string(REPLACE "powerpc64le" "ppc64le" _suffix ${suffix})
        # suffix will be something like '.cpython-35m-x86_64-linux-gnu.so'
        # result should be something like 'lib.linux-x86_64-3.5'
        string(REGEX REPLACE
            "\\..*([0-9])([0-9]).*-(.*)-(.*)-.*"
            "lib.\\4-\\3-\\1.\\2"
          _phylanx_platlib ${_suffix})
      endif()
    endif()

    # compose full path
    set(${result} "${CMAKE_CURRENT_BINARY_DIR}/build/${_phylanx_platlib}")

    phylanx_info("phylanx_get_python_extension_location: ${suffix} -> ${${result}}")

  endif()

endmacro()
