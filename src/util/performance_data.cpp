//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>
#include <phylanx/util/performance_data.hpp>

#include <hpx/include/agas.hpp>
#include <hpx/include/future.hpp>
#include <hpx/include/performance_counters.hpp>
#include <hpx/include/runtime.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    std::string enable_measurements(std::string const& primitive_instance)
    {
        using phylanx::execution_tree::primitives::primitive_component;

        hpx::id_type id =
            hpx::agas::resolve_name(hpx::launch::sync, primitive_instance);

        if (!id)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::util::enable_measurements",
                "given primitive name cannot be resolved by AGAS");
        }

        auto p = hpx::get_ptr<primitive_component>(hpx::launch::sync, id);

        p->enable_measurements();

        return primitive_instance;
    }

    std::vector<std::string> enable_measurements(
        std::vector<std::string> const& primitive_instances)
    {
        if (primitive_instances.empty())
        {
            return enable_measurements(
                hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*$*"));
        }

        using phylanx::execution_tree::primitives::primitive_component;

        std::vector<hpx::future<std::shared_ptr<primitive_component>>> primitives;
        primitives.reserve(primitive_instances.size());

        for (auto const& entry : primitive_instances)
        {
            hpx::future<hpx::id_type> f = hpx::agas::resolve_name(entry);
            primitives.emplace_back(f.then(
                [](hpx::future<hpx::id_type>&& f)
                ->  hpx::future<std::shared_ptr<primitive_component>>
                {
                    return hpx::get_ptr<primitive_component>(f.get());
                }));
        }

        hpx::wait_all(primitives);

        for (auto& f : primitives)
        {
            f.get()->enable_measurements();
        }

        return primitive_instances;
    }

    std::vector<std::string> enable_measurements(
        std::map<std::string, hpx::id_type> const& primitive_instances)
    {
        using phylanx::execution_tree::primitives::primitive_component;

        std::vector<std::string> result;
        result.reserve(primitive_instances.size());

        std::vector<hpx::future<std::shared_ptr<primitive_component>>> primitives;
        primitives.reserve(primitive_instances.size());

        for (auto const& entry : primitive_instances)
        {
            primitives.emplace_back(
                hpx::get_ptr<primitive_component>(entry.second));
            result.emplace_back(entry.first);
        }

        hpx::wait_all(primitives);

        for (auto& f : primitives)
        {
            f.get()->enable_measurements();
        }

        return result;
    }

    std::vector<std::string> enable_measurements()
    {
        return enable_measurements(
            hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*$*"));
    }

    ///////////////////////////////////////////////////////////////////////////
    std::map<std::string, std::vector<std::int64_t>> retrieve_counter_data(
        std::vector<std::string> const& primitive_instances,
        std::vector<std::string> const& counter_name_last_parts,
        hpx::naming::id_type const& locality_id)
    {
        // Ensure counter_name_last_part has at least one entry
        if (counter_name_last_parts.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "retrieve_counter_data",
                "counter_name_last_parts cannot be empty");
        }

        // NOTE: primitive_instances are not verified

        // Querying for performance counters is relatively expensive and there
        // is overlap, thus we can use futures
        //   key: primitive type
        //   value: future of vector of performance counter values
        std::map<std::string,
            std::vector<
                hpx::future<hpx::performance_counters::counter_values_array>>>
            values_futures_pile;

        // Iterate through all provided primitive instances
        for (auto const& name : primitive_instances)
        {
            // Parse the primitive name
            auto const tags =
                phylanx::execution_tree::compiler::parse_primitive_name(name);

            // Performance counter values
            std::vector<
                hpx::future<hpx::performance_counters::counter_values_array>>&
                futures = values_futures_pile[tags.primitive];

            if (futures.empty())
            {
                // Preallocate memory
                futures.reserve(counter_name_last_parts.size());

                // Iterate through the last parts of performance counter names
                for (auto const& counter_name_last_part :
                    counter_name_last_parts)
                {
                    // Construct the name of the counter
                    std::string const counter_name("/phylanx/primitives/" +
                        tags.primitive + "/" + counter_name_last_part);
                    // The actual performance counter
                    hpx::performance_counters::performance_counter counter(
                        counter_name, locality_id);
                    futures.push_back(counter.get_counter_values_array(false));
                }
            }
        }

        // We need the performance counter values. Wait until they are done
        for (auto& entry : values_futures_pile)
        {
            hpx::wait_all(entry.second);
        }

        // Reuse get_counter_values_array calls
        //   key: primitive type
        //   value: vector of performance counter values
        std::map<std::string, std::vector<std::vector<std::int64_t>>>
            counter_values_pile;

        // Collect the values from the futures
        for (auto& values_futures : values_futures_pile)
        {
            // Performance counter values
            std::vector<std::vector<std::int64_t>>& counter_values =
                counter_values_pile[values_futures.first];

            // Preallocate memory
            counter_values.reserve(counter_name_last_parts.size());

            // Collect the performance counter values
            for (auto& f : values_futures.second)
            {
                counter_values.push_back(f.get().values_);
            }
        }

        // collect range of tags for each primitive type
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> tag_ranges;

        // Iterate and collect the result from the futures
        constexpr std::int64_t maxval = (std::numeric_limits<std::int64_t>::max)();
        constexpr std::int64_t minval = (std::numeric_limits<std::int64_t>::min)();

        for (auto const& name : primitive_instances)
        {
            // Parse the primitive name
            auto const tags =
                phylanx::execution_tree::compiler::parse_primitive_name(name);

            auto it = tag_ranges.find(tags.primitive);
            if (it == tag_ranges.end())
            {
                auto p = tag_ranges.insert(decltype(tag_ranges)::value_type(
                    tags.primitive, std::make_pair(maxval, minval)));
                it = p.first;
            }

            it->second.first =
                (std::min)(it->second.first, tags.sequence_number);
            it->second.second =
                (std::max)(it->second.second, tags.sequence_number);
        }

        // Return value
        std::map<std::string, std::vector<std::int64_t>> result;

        // Iterate and collect the result from the futures
        for (auto const& name : primitive_instances)
        {
            // Parse the primitive name
            auto const tags =
                phylanx::execution_tree::compiler::parse_primitive_name(name);

            // Performance counter values
            std::vector<std::vector<std::int64_t>>& counter_values =
                counter_values_pile[tags.primitive];

            // extract offset for sequence number
            auto const& r = tag_ranges[tags.primitive];

            // Collect the performance counter values
            std::vector<std::int64_t> data(counter_name_last_parts.size());
            for (unsigned int i = 0; i < counter_values.size(); ++i)
            {
                HPX_ASSERT(static_cast<std::size_t>(r.second - r.first) <
                    counter_values[i].size());

                data[i] = counter_values[i][tags.sequence_number - r.first];
            }

            result.emplace(decltype(result)::value_type(name, data));
        }

        return result;
    }

    std::map<std::string, std::vector<std::int64_t>> retrieve_counter_data(
        std::vector<std::string> const& primitive_instances,
        hpx::naming::id_type const& locality_id)
    {
        std::vector<std::string> const counter_names{
            "count/eval", "time/eval", "eval_direct"
        };

        return retrieve_counter_data(
            primitive_instances, counter_names, locality_id);
    }

    std::map<std::string, std::vector<std::int64_t>> retrieve_counter_data(
        hpx::naming::id_type const& locality_id)
    {
        auto entries =
            hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*$*");

        std::vector<std::string> primitive_instances;
        primitive_instances.reserve(entries.size());

        for (auto&& entry : entries)
        {
            primitive_instances.emplace_back(std::move(entry.first));
        }

        return retrieve_counter_data(primitive_instances, locality_id);
    }
}}
