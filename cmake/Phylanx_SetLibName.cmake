# Copyright (c) 2015      John Biddiscombe
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#-------------------------------------------------------------------------------
# adds phylanx_ prefix to give phylanx_${name} to libraries and components
#-------------------------------------------------------------------------------
MACRO (phylanx_set_lib_name target name)
  # there is no need to change debug/release names explicitly
  # as we use CMAKE_DEBUG_POSTFIX to alter debug names

  hpx_debug("phylanx_set_lib_name: target:" ${target} "name: " ${name})
  set_target_properties (${target}
      PROPERTIES
      OUTPUT_NAME                phylanx_${name}
      DEBUG_OUTPUT_NAME          phylanx_${name}
      RELEASE_OUTPUT_NAME        phylanx_${name}
      MINSIZEREL_OUTPUT_NAME     phylanx_${name}
      RELWITHDEBINFO_OUTPUT_NAME phylanx_${name}
  )
ENDMACRO()
