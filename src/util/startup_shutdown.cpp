//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <hpx/include/components.hpp>
#include <hpx/runtime/startup_function.hpp>
#include <hpx/runtime/shutdown_function.hpp>

namespace phylanx
{
    namespace performance_counters
    {
        void startup_counters();
    }
}

namespace phylanx { namespace util
{
    phylanx::plugin::plugin_map_type plugin_map;

    ///////////////////////////////////////////////////////////////////////////
    void startup()
    {
        // plugins must be loaded first to register all external primitives
        plugin::load_plugins(plugin_map);

        // register performance counters for all discovered primitives
        performance_counters::startup_counters();
    }

    void shutdown()
    {
        // unload all plugin modules
        plugin_map.clear();
    }

    ///////////////////////////////////////////////////////////////////////////
    bool get_startup(hpx::startup_function_type& startup_func,
        bool& pre_startup)
    {
        // return our startup-function
        startup_func = startup;   // function to run during startup
        pre_startup = true;                 // run as pre-startup function
        return true;
    }

    bool get_shutdown(hpx::shutdown_function_type& shutdown_func,
        bool& pre_shutdown)
    {
        // return our startup-function
        shutdown_func = shutdown;   // function to run during startup
        pre_shutdown = false;                 // run as pre-startup function
        return true;
    }
}}

// Note that this macro can be used not more than once in one module.
HPX_REGISTER_STARTUP_SHUTDOWN_MODULE(phylanx::util::get_startup,
    phylanx::util::get_shutdown);
