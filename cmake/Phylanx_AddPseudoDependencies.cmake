# Copyright (c) 2011 Bryce Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

macro(add_phylanx_pseudo_dependencies)

  if("${PHYLANX_CMAKE_LOGLEVEL}" MATCHES "DEBUG|debug|Debug")
    set(args)
    foreach(arg ${ARGV})
      set(args "${args} ${arg}")
    endforeach()
    phylanx_debug("add_phylanx_pseudo_dependencies" ${args})
  endif()

  if(PHYLANX_WITH_PSEUDO_DEPENDENCIES)
    set(shortened_args)
    foreach(arg ${ARGV})
      shorten_phylanx_pseudo_target(${arg} shortened_arg)
      set(shortened_args ${shortened_args} ${shortened_arg})
    endforeach()
    add_dependencies(${shortened_args})
  endif()
endmacro()

macro(add_phylanx_pseudo_dependencies_no_shortening)

  if("${PHYLANX_CMAKE_LOGLEVEL}" MATCHES "DEBUG|debug|Debug")
    set(args)
    foreach(arg ${ARGV})
      set(args "${args} ${arg}")
    endforeach()
    phylanx_debug("add_phylanx_pseudo_dependencies_no_shortening" ${args})
  endif()

  if(PHYLANX_WITH_PSEUDO_DEPENDENCIES)
    add_dependencies(${ARGV})
  endif()
endmacro()

