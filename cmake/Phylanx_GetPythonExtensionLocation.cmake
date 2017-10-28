# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

macro(phylanx_get_python_extension_location suffix result)

  if(${suffix} STREQUAL ".so")
	message(SEND_ERROR "No .so file found in Python libraray, 
		Phylanx does not support static libraries,  
		Phylanx found ${PYTHON_LIBRARIES} ")

  else()	
	# extract parts of suffix and generate needed output structure
  	if(MSVC)
		# suffix will be something like '.cp36-win_amd64.pyd'
		# result should be something like 'lib.win-amd64-3.6'
		string(REGEX REPLACE
		      "\\.cp([0-9])([0-9])-(.*)_(.*)\\..*"
		      "lib.\\3-\\4-\\1.\\2"
	      _result ${suffix})

	else()
        # Pybind gets the architecture wrong for POWER8.
        string(REPLACE "powerpc64le" "ppc64le" _suffix ${suffix})
		# suffix will be something like '.cpython-35m-x86_64-linux-gnu.so'
		# result should be something like 'lib.linux-x86_64-3.5'
		string(REGEX REPLACE
			"\\..*([0-9])([0-9]).*-(.*)-(.*)-.*"
			"lib.\\4-\\3-\\1.\\2"
		_result ${_suffix})
	endif()

	# compose full path
	set(${result} "${CMAKE_CURRENT_BINARY_DIR}/build/${_result}")

	phylanx_info("phylanx_get_python_extension_location: ${suffix} -> ${${result}}")

  endif()

endmacro()
