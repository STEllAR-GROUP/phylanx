//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/compile.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/performance_counters.hpp>
#include <hpx/include/util.hpp>
#include <hpx/performance_counters/base_performance_counter.hpp>
#include <hpx/runtime/startup_function.hpp>

#include <cstdint>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace performance_counters {
    class primitive_counter
      : public hpx::performance_counters::base_performance_counter<primitive_counter>
    {
    public:
        primitive_counter() = default;

        primitive_counter(
            hpx::performance_counters::counter_info const& info)
          : hpx::performance_counters::base_performance_counter<
                primitive_counter>(info)
        { }

        hpx::performance_counters::counter_values_array
        get_counter_values_array(bool reset) override
        {
            hpx::performance_counters::counter_values_array value;

            value.time_ = hpx::util::high_resolution_clock::now();
            value.status_ = hpx::performance_counters::status_new_data;
            value.count_ = ++invocation_count_;

            /*value.values_ = std::move(result);*/

            return value;
        }

        // Retrieve the list of existing primitives for the current execution tree and keep it
        void reinit(bool reset) override
        {
            namespace et = phylanx::execution_tree;

            et::pattern_list const& pattern_list = et::get_all_known_patterns();

            for (auto const& patterns : pattern_list)
            {
                auto const& pattern = *(patterns.begin());
                std::string const& name = hpx::util::get<0>(pattern);
                auto entries = hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/name#*");
            }
        }
    };

    using primitive_counter_type = hpx::components::component<primitive_counter>;

    hpx::naming::gid_type primitive_counter_creator(
        hpx::performance_counters::counter_info const& info,
        hpx::error_code& ec)
    {
        namespace pc = hpx::performance_counters;

        // Break down the counter name
        pc::counter_path_elements paths;
        pc::get_counter_path_elements(info.fullname_, paths, ec);
        if (ec)
            return hpx::naming::invalid_gid;

        // If another counter's name was give
        if (paths.parentinstance_is_basename_)
        {
            HPX_THROWS_IF(ec,
                hpx::bad_parameter,
                "primitive_counter_creator",
                "invalid counter instance parent name: " +
                paths.parentinstancename_);
            return hpx::naming::invalid_gid;
        }

        if (paths.instancename_ == "total" && paths.instanceindex_ == -1)
        {
            pc::counter_info complemented_info = info;
            pc::complement_counter_info(complemented_info, info, ec);
            if (ec)
                return hpx::naming::invalid_gid;

            hpx::naming::gid_type id;
            try
            {
                // Try constructing the actual counter
                id = hpx::components::server::construct<primitive_counter_type>(
                    complemented_info);
            }
            catch (hpx::exception const& e)
            {
                if (&ec == &hpx::throws)
                    throw;
                ec = hpx::make_error_code(e.get_error(), e.what());
                return hpx::naming::invalid_gid;
            }

            if (&ec != &hpx::throws)
                ec = hpx::make_success_code();
            return id;
        }

        HPX_THROWS_IF(ec,
            hpx::bad_parameter,
            "primitive_counter_creator",
            "invalid counter instance name: " + paths.instancename_);
        return hpx::naming::invalid_gid;
    }

    ///////////////////////////////////////////////////////////////////////////
    // This function will be registered as a startup function for HPX below.
    // That means it will be executed in an HPX-thread before hpx_main, but
    // after the runtime has been initialized and started.
    void startup()
    {
        // Install the counter types, de-installation of the types is handled
        // automatically.
        hpx::performance_counters::install_counter_type(
            "/phylanx/count/node_data/copy_constructions",
            &ir::node_data<double>::copy_construction_count,
            "returns the current value of the copy-construction count of "
                "any node_data<double>");

        hpx::performance_counters::install_counter_type(
            "/phylanx/count/node_data/move_constructions",
            &ir::node_data<double>::move_construction_count,
            "returns the current value of the move-construction count of "
                "any node_data<double>");

        hpx::performance_counters::install_counter_type(
            "/phylanx/count/node_data/copy_assignments",
            &ir::node_data<double>::copy_assignment_count,
            "returns the current value of the copy-assignment count of "
                "any node_data<double>");

        hpx::performance_counters::install_counter_type(
            "/phylanx/count/node_data/move_assignments",
            &ir::node_data<double>::move_assignment_count,
            "returns the current value of the move-assignment count of "
                "any node_data<double>");


        namespace et = phylanx::execution_tree;

        // Query the list of existing primitive types
        et::pattern_list const& pattern_list = et::get_all_known_patterns();

        // Iterate and register a time and count performance counter per each primitive
        for (auto const& patterns : pattern_list)
        {
            // No need to process all entries in the list
            auto const& pattern = *(patterns.begin());
            // The name of the primitive
            std::string const& name = hpx::util::get<0>(pattern);
            // Register a primitive time performance counter
            hpx::performance_counters::install_counter_type(
                "/phylanx/time/primitives/" + name + "/eval",
                hpx::performance_counters::counter_raw_values,
                "returns the total execution time of eval() function of the " +
                    name + " primitive",
                &primitive_counter_creator,
                &hpx::performance_counters::locality_counter_discoverer);
            // Register a primitive count performance counter
            hpx::performance_counters::install_counter_type(
                "/phylanx/count/primitives/" + name + "/eval",
                hpx::performance_counters::counter_raw_values,
                "returns the number of times of eval() function of each " +
                name + " primitive instance was called",
                &primitive_counter_creator,
                &hpx::performance_counters::locality_counter_discoverer);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    bool get_startup(hpx::startup_function_type& startup_func,
        bool& pre_startup)
    {
        // return our startup-function if performance counters are required
        startup_func = startup;   // function to run during startup
        pre_startup = true;       // run 'startup' as pre-startup function
        return true;
    }
}}

///////////////////////////////////////////////////////////////////////////////
// Register a startup function that will be called as a HPX-thread during
// runtime startup. We use this function to register our performance counter
// type and performance counter instances.
HPX_REGISTER_STARTUP_MODULE(phylanx::performance_counters::get_startup);

using primitive_counter_type = hpx::components::component<
    phylanx::performance_counters::primitive_counter>;
using primitive_counter = phylanx::performance_counters::primitive_counter;

HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    primitive_counter_type, primitive_counter, "base_performance_counter");

