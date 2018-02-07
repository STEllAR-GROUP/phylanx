//   Copyright (c) 2018 Parsa Amini
//   Copyright (c) 2018 Hartmut Kaiser
//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/agas.hpp>
#include <hpx/runtime_fwd.hpp>

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>

///////////////////////////////////////////////////////////////////////////////
char const* const fib_code = R"(block(
    define(fib_test, iterations,
        block(
            define(x, 1.0),
            define(z, 0.0),
            define(y, 1.0),
            cout(x),
            cout(y),
            define(temp,0.0),
            define(step, 0),
            while(
                step < iterations,
                block(
                    store(z, x + y),
                    store(temp, y),
                    store(y, z),
                    store(x, temp),
                    cout(z),
                    store(step, step + 1)
                )
            )
        )
    ),
    fib_test
))";

// Retrieve performance counter data for selected primitives
//   primitive_instances:     The primitives for which performance counter data
//                            is required
//   locality_id:             The locality the performance counter data is going
//                            to be queried from
//   counter_name_last_parts: A vector containing the last part of the
//                            performance counter names.
//                            e.g. std::vector{ "count/eval", "count/direct_eval" }
std::map<std::string, std::vector<std::int64_t>> retrieve_counter_data(
    std::vector<std::string> const& primitive_instances,
    std::vector<std::string> const& counter_name_last_parts,
    hpx::naming::id_type const& locality_id = hpx::find_here())
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
        auto tags =
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
            for (auto const& counter_name_last_part : counter_name_last_parts)
            {
                // Construct the name of the counter
                std::string counter_name("/phylanx/primitives/" +
                    tags.primitive + "/" + counter_name_last_part);
                // The actual performance counter
                hpx::performance_counters::performance_counter counter(
                    counter_name, locality_id);
                futures.push_back(
                    counter.get_counter_values_array(false));
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
        for (int i = 0; i < counter_values.size(); ++i)
        {
            data[i] = counter_values[i][tags.sequence_number];
        }

        result.emplace(decltype(result)::value_type(name, data));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(boost::program_options::variables_map& vm)
{
    // Compile the given code
    phylanx::execution_tree::compiler::function_list snippets;

    auto fibonacci = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(fib_code), snippets);

    // Fibonacci arguments
    auto num_iterations = vm["num_iterations"].as<std::int64_t>();

    // Evaluate Fibonacci using the read data
    fibonacci(num_iterations);

    // CSV Header
    std::cout << "primitive_instance" << ","
        << "count" << ","
        << "time" << ","
        << "direct_count" << ","
        << "direct_time"
        << std::endl;

    // List of existing primitive instances
    std::vector<std::string> existing_primitive_instances;

    // Retrieve all primitive instances
    for (auto const& entry :
        hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*#*"))
    {
        existing_primitive_instances.push_back(entry.first);
    }

    // Print performance data
    for (auto const& entry :
        retrieve_counter_data(existing_primitive_instances,
            std::vector<std::string>{"count/eval", "time/eval",
                "count/eval_direct", "time/eval_direct"}, hpx::find_here()))
    {
        std::cout << "\"" << entry.first << "\"";
        for (auto const& counter_value : entry.second)
        {
            std::cout << "," << counter_value;
        }
        std::cout << std::endl;
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // Command-line handling
    boost::program_options::options_description desc(
        "usage: primitive_instrumentation [options]");
    desc.add_options()
        ("num_iterations,n",
            boost::program_options::value<std::int64_t>()->default_value(10),
            "number of iterations (default: 10)");

    return hpx::init(desc, argc, argv);
}
