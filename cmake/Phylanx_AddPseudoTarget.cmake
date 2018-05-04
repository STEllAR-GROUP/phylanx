# Copyright (c) 2011 Bryce Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(HPX_ADDPSEUDOTARGET_LOADED TRUE)

macro(add_phylanx_pseudo_target_always)
  set(shortened_args)
  foreach(arg ${ARGV})
    shorten_hpx_pseudo_target(${arg} shortened_arg)
    set(shortened_args ${shortened_args} ${shortened_arg})
  endforeach()
  phylanx_debug("add_phylanx_pseudo_target" "adding shortened pseudo target: ${shortened_args}")
  add_custom_target(${shortened_args})
endmacro()

macro(add_phylanx_pseudo_target)
  phylanx_debug("add_phylanx_pseudo_target" "adding pseudo target: ${ARGV}")
  if(PHYLANX_WITH_PSEUDO_DEPENDENCIES)
    add_phylanx_pseudo_target_always(${ARGV})
  endif()
endmacro()

