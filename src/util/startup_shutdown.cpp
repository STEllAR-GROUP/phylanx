//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/plugins/plugin_factory.hpp>
#include <phylanx/util/performance_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/modules/errors.hpp>
#include <hpx/modules/performance_counters.hpp>
#include <hpx/modules/runtime_local.hpp>

#include <fstream>
#include <iostream>
#include <string>

namespace phylanx {

    namespace performance_counters {

        void startup_counters();
    }

    namespace util {

        bool need_performance_counters(std::string& filename);
    }
}    // namespace phylanx

namespace phylanx { namespace util {

    phylanx::plugin::plugin_map_type plugin_map;

    bool print_performance_counter_data = false;
    std::string performance_counter_dest;

    bool need_performance_counters()
    {
        return print_performance_counter_data;
    }

    ///////////////////////////////////////////////////////////////////////////
    void startup()
    {
        // plugins must be loaded first to register all external primitives
        plugin::load_plugins(plugin_map);

        // register performance counters for all discovered primitives
        performance_counters::startup_counters();

        // enable performance counters if requested on command line
        if (need_performance_counters(performance_counter_dest))
        {
            phylanx::util::enable_measurements();
            hpx::reinit_active_counters();
            print_performance_counter_data = true;
        }
    }

    void print_performance_counter_data_csv(std::ostream& os)
    {
        // CSV Header
        os << "primitive_instance,display_name,count,time,eval_direct\n";

        // Print performance data
        for (auto const& entry : phylanx::util::retrieve_counter_data())
        {
            os << "\"" << entry.first << "\",\""
               << phylanx::execution_tree::compiler::primitive_display_name(
                      entry.first)
               << "\"";
            for (auto const& counter_value : entry.second)
            {
                os << "," << counter_value;
            }
            os << "\n";
        }

        os << "\n";
    }

    void shutdown()
    {
        // print performance counter data, if requested
        if (print_performance_counter_data)
        {
            if (performance_counter_dest.empty())
            {
                print_performance_counter_data_csv(std::cout);
            }
            else
            {
                std::ofstream os(performance_counter_dest);
                if (!os.good())
                {
                    HPX_THROW_EXCEPTION(hpx::filesystem_error,
                        "phylanx::util::shutdown",
                        "Failed to open the specified file: " +
                            performance_counter_dest);
                }

                print_performance_counter_data_csv(os);
            }
        }

        // unload all plugin modules
        plugin_map.clear();
    }

    ///////////////////////////////////////////////////////////////////////////
    bool get_startup(
        hpx::startup_function_type& startup_func, bool& pre_startup)
    {
        // return our startup-function
        startup_func = startup;    // function to run during startup
        pre_startup = true;        // run as pre-startup function
        return true;
    }

    bool get_shutdown(
        hpx::shutdown_function_type& shutdown_func, bool& pre_shutdown)
    {
        // return our startup-function
        shutdown_func = shutdown;    // function to run during shutdown
        pre_shutdown = false;        // run as pre-shutdown function
        return true;
    }
}}    // namespace phylanx::util

// Note that this macro can't be used more than once in one module.
HPX_REGISTER_STARTUP_SHUTDOWN_MODULE(
    phylanx::util::get_startup, phylanx::util::get_shutdown);
