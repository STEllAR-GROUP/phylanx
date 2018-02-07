//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/util/instrumentation/performance_data.hpp>

#include <hpx/include/future.hpp>
#include <hpx/include/performance_counters.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace phylanx { namespace util
{
    std::map<std::string, std::vector<std::int64_t>> retrieve_counter_data(
        std::vector<std::string> const& primitive_instances,
        std::vector<std::string> const& counter_name_last_parts,
        hpx::naming::id_type const& locality_id)
    {
        // Return value
        std::map<std::string, std::vector<std::int64_t>> result;

        // Reuse get_counter_values_array calls
        //   key: primitive type
        //   value: vector of performance counter values
        std::map<std::string, std::vector<std::vector<std::int64_t>>>
            counter_values_pile;

        // NOTE: primitive_instances are not verified

        // Iterate through all provided primitive instances
        for (auto const& name : primitive_instances)
        {
            // Parse the primitive name
            auto const tags =
                phylanx::execution_tree::compiler::parse_primitive_name(name);

            // Ensure counter_name_last_part has at least one entry
            if (counter_name_last_parts.empty())
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "retrieve_counter_data",
                    "counter_name_last_parts cannot be empty");
            }

            // Performance counter values
            std::vector<std::vector<std::int64_t>>& counter_values =
                counter_values_pile[tags.primitive];

            // Querying for performance counters is relatively expensive and there
            // is overlap, thus we can use futures
            std::vector<
                hpx::future<hpx::performance_counters::counter_values_array>>
                futures;

            if (counter_values.empty())
            {
                // Preallocate memory
                counter_values.reserve(counter_name_last_parts.size());

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

            // We need the performance counter values. Wait until they are done
            hpx::wait_all(futures);

            // Collect the performance counter values
            for (auto& f : futures)
            {
                counter_values.push_back(f.get().values_);
            }

            std::vector<std::int64_t> data(counter_name_last_parts.size());
            for (unsigned int i = 0; i < counter_values.size(); ++i)
            {
                data[i] = counter_values[i][tags.sequence_number];
            }

            result.emplace(decltype(result)::value_type(name, data));
        }

        return result;
    }
}}
