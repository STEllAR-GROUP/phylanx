//  Copyright (c) 2016-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PLUGIN_BASE_APR_06_2018_1249PM)
#define PHYLANX_PLUGIN_BASE_APR_06_2018_1249PM

#include <phylanx/config.hpp>

#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace plugin
{
    struct plugin_base
    {
        virtual ~plugin_base() {}
        virtual void register_known_primitives(std::string const& fullpath) = 0;
    };
}}

#endif

