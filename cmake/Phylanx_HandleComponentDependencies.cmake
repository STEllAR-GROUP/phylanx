# Copyright (c) 2007-2015 Hartmut Kaiser
# Copyright (c) 2011      Bryce Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

macro(phylanx_handle_component_dependencies components)
  set(tmp "")

  foreach(component ${${components}})
    if(TARGET ${component}_component)
      set(tmp ${tmp} "general;${component}_component")
    elseif(TARGET HPX::${component}_component)
      set(tmp ${tmp} HPX::${component}_component)
    else()
      set(tmp ${tmp} hpx_${component})
    endif()
    phylanx_debug("phylanx_handle_component_dependencies: ${tmp}")
  endforeach()

  set(${components} ${tmp})
endmacro()

