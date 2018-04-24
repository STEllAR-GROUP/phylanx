//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_CONFIG_USER_MAIN_CONFIG)
#define PHYLANX_CONFIG_USER_MAIN_CONFIG

#include <hpx/hpx_init.hpp>

#include <vector>
#include <string>

namespace phylanx { namespace detail
{
    // Allow applications to add configuration settings if HPX_MAIN is set
    PHYLANX_EXPORT std::vector<std::string> user_main_config(
        std::vector<std::string> const& config);

    // Make sure our configuration information is injected into the startup
    // procedure
    struct PHYLANX_EXPORT register_user_main_config
    {
        register_user_main_config();
    };

    PHYLANX_EXPORT extern register_user_main_config cfg;
}}

#endif
