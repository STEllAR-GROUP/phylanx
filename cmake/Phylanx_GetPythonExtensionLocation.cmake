# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# suffix will be something like '.cp36-win_amd64.pyd'
# result should be something like 'lib.win-amd64-3.6'
macro(phylanx_get_python_extension_location suffix result)

  # extract parts of suffix and generate needed output structure
  string(REGEX REPLACE
    "\\.cp([0-9])([0-9])-(.*)_(.*)\\..*"
    "lib.\\3-\\4-\\1.\\2"
    _result ${suffix})

  # compose full path
  set(${result} "${CMAKE_CURRENT_BINARY_DIR}/build/${_result}")

endmacro()
