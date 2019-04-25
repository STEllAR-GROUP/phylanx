//  Copyright (c) 2018 Hartmut Kaiser
//  Copyright (c) 2018 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compile.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/agas.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/performance_counters.hpp>
#include <hpx/include/util.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <map>

#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_difference.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_plus.hpp>
#include <boost/spirit/include/qi_sequence.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace performance_counters
{
    namespace detail
    {
        std::string extract_primitive_type(
            hpx::performance_counters::counter_info const& info)
        {
            hpx::performance_counters::counter_path_elements paths;
            hpx::performance_counters::get_counter_path_elements(
                info.fullname_, paths);

            std::string result;

            namespace qi = boost::spirit::qi;
            bool success =
                qi::parse(paths.countername_.begin(), paths.countername_.end(),
                    "primitives/" >> +(qi::char_ - '/'), result);

            if (!success)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "performance_counter::detail::extract_primitive_type",
                    "unexpected counter name");
            }

            return result;
        }
    }

    class primitive_counter
      : public hpx::performance_counters::base_performance_counter<
            primitive_counter>
    {
    public:
        primitive_counter()
          : first_init_(false)
          , duration_counter_(false)
        {}

        primitive_counter(hpx::performance_counters::counter_info const& info)
          : hpx::performance_counters::base_performance_counter<
                primitive_counter>(info)
          , first_init_(false)
          , duration_counter_(false)
        {
            hpx::performance_counters::counter_path_elements paths;
            hpx::performance_counters::get_counter_path_elements(
                info.fullname_, paths);
            duration_counter_ =
                paths.countername_.find("time") != std::string::npos;
        }

        // Produce the counter value
        hpx::performance_counters::counter_values_array
        get_counter_values_array(bool reset) override
        {
            // Need to call reinit here if it has never been called before.
            bool expected = false;
            if (first_init_.compare_exchange_strong(expected, true))
            {
                reinit(false);
            }

            hpx::performance_counters::counter_values_array value;

            value.time_ = static_cast<std::int64_t>(hpx::get_system_uptime());
            value.status_ = hpx::performance_counters::status_new_data;
            value.count_ = ++invocation_count_;

            std::vector<std::int64_t> result;
            result.reserve(instances_.size());

            // Extract the values from instances_
            for (auto const& instance : instances_)
            {
                if (duration_counter_)
                {
                    result.push_back(instance->get_eval_duration(reset));
                }
                else
                {
                    result.push_back(instance->get_eval_count(reset));
                }
            }

            value.values_ = std::move(result);

            return value;
        }

        // Retrieve the list of existing primitives for the current execution
        // tree and keep it
        void reinit(bool reset) override
        {
            // Structure of primitives in symbolic namespace:
            //     /phylanx/<primitive>$<sequence-nr>[$<instance>]/<compile_id>$<tag>
            auto entries = hpx::agas::find_symbols(hpx::launch::sync,
                "/phylanx/" + detail::extract_primitive_type(info_) + "$*");

            // TODO: Only keep entries that live on this locality.
            // This will be a problem when Phylanx becomes distributed.
            std::map<std::int64_t, base_primitive_ptr> instances_sorted;

            for (auto const& value : entries)
            {
                auto const& instance = hpx::get_ptr<
                    phylanx::execution_tree::primitives::primitive_component>(
                        hpx::launch::sync, value.second);

                auto instance_info =
                    phylanx::execution_tree::compiler::parse_primitive_name(
                        value.first);

                instance->enable_measurements();

                // Consider the reset flag
                if (reset)
                {
                    if (duration_counter_)
                    {
                        instance->get_eval_duration(true);
                    }
                    else
                    {
                        instance->get_eval_count(true);
                    }
                }
                instances_sorted[instance_info.sequence_number] = instance;
            }

            instances_.clear();
            instances_.reserve(entries.size());

            for (auto && value : instances_sorted)
            {
                instances_.emplace_back(std::move(value.second));
            }

            first_init_ = true;
        }

    private:
        using base_primitive_ptr = std::shared_ptr<
            phylanx::execution_tree::primitives::primitive_component>;

        std::vector<base_primitive_ptr> instances_;
        std::atomic<bool> first_init_;
        bool duration_counter_;
    };

    hpx::naming::gid_type primitive_counter_creator(
        hpx::performance_counters::counter_info const& info,
        hpx::error_code& ec)
    {
        namespace pc = hpx::performance_counters;

        // Break down the counter name
        pc::counter_path_elements paths;
        pc::get_counter_path_elements(info.fullname_, paths, ec);
        if (ec) return hpx::naming::invalid_gid;

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
            if (ec) return hpx::naming::invalid_gid;

            hpx::naming::gid_type id;
            try
            {
                // Try constructing the actual counter
                using primitive_counter_type =
                    hpx::components::component<primitive_counter>;

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
    class direct_execution_counter
      : public hpx::performance_counters::base_performance_counter<
            direct_execution_counter>
    {
    public:
        direct_execution_counter()
          : first_init_(false)
        {}

        direct_execution_counter(
                hpx::performance_counters::counter_info const& info)
          : hpx::performance_counters::base_performance_counter<
                direct_execution_counter>(info)
          , first_init_(false)
        {}

        // Produce the counter value
        hpx::performance_counters::counter_values_array
        get_counter_values_array(bool reset) override
        {
            // Need to call reinit here if it has never been called before.
            bool expected = false;
            if (first_init_.compare_exchange_strong(expected, true))
            {
                reinit(false);
            }

            hpx::performance_counters::counter_values_array value;

            value.time_ = static_cast<std::int64_t>(hpx::get_system_uptime());
            value.status_ = hpx::performance_counters::status_new_data;
            value.count_ = ++invocation_count_;

            std::vector<std::int64_t> result;
            result.reserve(instances_.size());

            // Extract the values from instances_
            for (auto const& instance : instances_)
            {
                result.push_back(instance->get_direct_execution(reset));
            }

            value.values_ = std::move(result);

            return value;
        }

        // Retrieve the list of existing primitives for the current execution
        // tree and keep it
        void reinit(bool reset) override
        {
            // Structure of primitives in symbolic namespace:
            //     /phylanx/<primitive>$<sequence-nr>[$<instance>]/<compile_id>$<tag>
            auto entries = hpx::agas::find_symbols(hpx::launch::sync,
                "/phylanx/" + detail::extract_primitive_type(info_) + "$*");

            // TODO: Only keep entries that live on this locality.
            // This will be a problem when Phylanx becomes distributed.
            std::map<std::int64_t, base_primitive_ptr> instances_sorted;

            for (auto const& value : entries)
            {
                auto const& instance = hpx::get_ptr<
                    phylanx::execution_tree::primitives::primitive_component>(
                        hpx::launch::sync, value.second);

                auto instance_info =
                    phylanx::execution_tree::compiler::parse_primitive_name(
                        value.first);

                // Consider the reset flag
                if (reset)
                {
                    instance->get_direct_execution(true);
                }
                instances_sorted[instance_info.sequence_number] = instance;
            }

            instances_.clear();
            instances_.reserve(entries.size());
            for (auto const& value : instances_sorted)
            {
                instances_.push_back(value.second);
            }

            first_init_ = true;
        }

    private:
        using base_primitive_ptr = std::shared_ptr<
            phylanx::execution_tree::primitives::primitive_component>;

        std::vector<base_primitive_ptr> instances_;
        std::atomic<bool> first_init_;
    };

    hpx::naming::gid_type direct_execution_counter_creator(
        hpx::performance_counters::counter_info const& info,
        hpx::error_code& ec)
    {
        namespace pc = hpx::performance_counters;

        // Break down the counter name
        pc::counter_path_elements paths;
        pc::get_counter_path_elements(info.fullname_, paths, ec);
        if (ec) return hpx::naming::invalid_gid;

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
            if (ec) return hpx::naming::invalid_gid;

            hpx::naming::gid_type id;
            try
            {
                // Try constructing the actual counter
                using direct_execution_counter_type =
                    hpx::components::component<direct_execution_counter>;

                id = hpx::components::server::construct<
                    direct_execution_counter_type>(complemented_info);
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
    void startup_counters()
    {
        // Install the counter types, de-installation of the types is handled
        // automatically.
        hpx::performance_counters::install_counter_type(
            "/phylanx/node_data_double/count/copy_constructions",
            &ir::node_data<double>::copy_construction_count,
            "returns the current value of the copy-construction count of "
                "any node_data<double>");

        hpx::performance_counters::install_counter_type(
            "/phylanx/node_data_double/count/move_constructions",
            &ir::node_data<double>::move_construction_count,
            "returns the current value of the move-construction count of "
                "any node_data<double>");

        hpx::performance_counters::install_counter_type(
            "/phylanx/node_data_double/count/copy_assignments",
            &ir::node_data<double>::copy_assignment_count,
            "returns the current value of the copy-assignment count of "
                "any node_data<double>");

        hpx::performance_counters::install_counter_type(
            "/phylanx/node_data_double/count/move_assignments",
            &ir::node_data<double>::move_assignment_count,
            "returns the current value of the move-assignment count of "
                "any node_data<double>");

        // Iterate and register a time and count performance counter per each
        // primitive
        namespace et = phylanx::execution_tree;
        for (auto const& pattern : et::get_all_known_patterns())
        {
            // The name of the primitive
            std::string const& name = pattern.data_.primitive_type_;

            // Register a primitive time performance counter
            hpx::performance_counters::install_counter_type(
                "/phylanx/primitives/" + name + "/time/eval",
                hpx::performance_counters::counter_raw_values,
                "returns a list whose elements contain the total execution "
                    "time of the eval function for each " +
                    name + " primitive",
                &primitive_counter_creator,
                &hpx::performance_counters::locality_counter_discoverer,
                HPX_PERFORMANCE_COUNTER_V1, "ns");

            // Register a primitive count performance counter
            hpx::performance_counters::install_counter_type(
                "/phylanx/primitives/" + name + "/count/eval",
                hpx::performance_counters::counter_raw_values,
                "returns a list whose elements contain the number of times "
                    "the eval function was called for each " +
                    name + " primitive",
                &primitive_counter_creator,
                &hpx::performance_counters::locality_counter_discoverer);

            // Register a direct_execution performance counter
            hpx::performance_counters::install_counter_type(
                "/phylanx/primitives/" + name + "/eval_direct",
                hpx::performance_counters::counter_raw_values,
                "returns a list whose elements contain whether "
                    "the eval function for the " + name + " primitive "
                    "was executed directly",
                &direct_execution_counter_creator,
                &hpx::performance_counters::locality_counter_discoverer);
        }
    }
}}

///////////////////////////////////////////////////////////////////////////////
// Register the factory functionality for the performance counter
using primitive_counter_type = hpx::components::component<
    phylanx::performance_counters::primitive_counter>;
using primitive_counter = phylanx::performance_counters::primitive_counter;

HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    primitive_counter_type, primitive_counter, "base_performance_counter");

using direct_execution_type = hpx::components::component<
    phylanx::performance_counters::direct_execution_counter>;
using direct_execution_counter =
    phylanx::performance_counters::direct_execution_counter;

HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    direct_execution_type, direct_execution_counter, "base_performance_counter");
