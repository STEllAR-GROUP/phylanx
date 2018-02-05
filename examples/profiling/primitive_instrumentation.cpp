//   Copyright (c) 2018 Parsa Amini
//   Copyright (c) 2018 Hartmut Kaiser
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

///////////////////////////////////////////////////////////////////////////////
char const* const lra_code = R"(block(
    //
    // Logistic regression analysis algorithm
    //
    //   x: [30, 2]
    //   y: [30]
    define(lra, x, y, alpha,
        block(
            define(weights, constant(0.0, shape(x, 1))),     // weights: [2]
            define(transx, transpose(x)),                    // transx:  [2, 30]
            define(pred, constant(0.0, shape(x, 0))),
            define(error, constant(0.0, shape(x, 0))),
            define(gradient, constant(0.0, shape(x, 1))),
            define(step, 0),
            while(
                step < 10,
                block(
                    store(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),
                    store(error, pred - y),                  // error: [30]
                    store(gradient, dot(transx, error)),     // gradient: [2]
                    parallel_block(
                        store(weights, weights - (alpha * gradient)),
                        store(step, step + 1)
                    )
                )
            ),
            weights
        )
    ),
    lra
))";

// Retrieve performance counter data for selected primitives
//   primitive_instances:     The primitives for which performance counter data
//                            is required
//   locality_id:             The locality the performance counter data is going
//                            to be queried from
//   counter_name_last_parts: A vector containing the last part of the
//                            performance counter names.
//                            e.g. std::vector{ "count/eval", "count/direct_eval" }
// NOTE: primitive_instances are not verified
std::map<std::string, std::vector<std::int64_t>> retrieve_counter_data(
    std::vector<std::string> const& primitive_instances,
    hpx::naming::id_type locality_id,
    std::vector<std::string>
        counter_name_last_parts)
{
    // Return value
    std::map<std::string, std::vector<std::int64_t>> result;

    // Reuse get_counter_values_array calls
    //   key: primitive type
    //   value: vector of performance counter values
    std::map<std::string, std::vector<std::vector<std::int64_t>>>
        counter_values_pile;

    // Iterate through all provided primitive instances
    for (auto const& name : primitive_instances)
    {
        // Parse the primitive name
        auto tags =
            phylanx::execution_tree::compiler::parse_primitive_name(name);

        // TODO: Ensure counter_name_last_part has at least one entry

        // Performance counter values
        std::vector<std::vector<std::int64_t>>& counter_values =
            counter_values_pile[tags.primitive];
        if (counter_values.empty())
        {
            // Iterate through the last parts of performance counter names
            for (auto const& counter_name_last_part : counter_name_last_parts)
            {
                // Construct the name of the counter
                std::string counter_name("/phylanx/primitives/" +
                    tags.primitive + "/" + counter_name_last_part);
                // The actual performance counter
                hpx::performance_counters::performance_counter counter(
                    counter_name, locality_id);
                counter_values.push_back(
                    counter.get_counter_values_array(hpx::launch::sync, false)
                        .values_);
            }
        }

        std::vector<std::int64_t> data(counter_name_last_parts.size());
        for (int i = 0; i < counter_values.size(); ++i)
        {
            data[i] = counter_values[i][tags.sequence_number];
        }

        result[name] = data;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main()
{
    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;

    auto lra = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(lra_code), snippets);

    // print instrumentation information, if enabled
    auto entries =
        hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*");

    // LRA arguments

    blaze::DynamicMatrix<double> v1{ { 15.04, 16.74 },{ 13.82, 24.49 },
    { 12.54, 16.32 },{ 23.09, 19.83 },{ 9.268, 12.87 },{ 9.676, 13.14 },
    { 12.22, 20.04 },{ 11.06, 17.12 },{ 16.3 , 15.7 },{ 15.46, 23.95 },
    { 11.74, 14.69 },{ 14.81, 14.7 },{ 13.4 , 20.52 },{ 14.58, 13.66 },
    { 15.05, 19.07 },{ 11.34, 18.61 },{ 18.31, 20.58 },{ 19.89, 20.26 },
    { 12.88, 18.22 },{ 12.75, 16.7 },{ 9.295, 13.9 },{ 24.63, 21.6 },
    { 11.26, 19.83 },{ 13.71, 18.68 },{ 9.847, 15.68 },{ 8.571, 13.1 },
    { 13.46, 18.75 },{ 12.34, 12.27 },{ 13.94, 13.17 },{ 12.07, 13.44 } };

    blaze::DynamicVector<double> v2{ 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 };
    auto x = phylanx::ir::node_data<double>{ v1 };
    auto y = phylanx::ir::node_data<double>{ v2 };
    auto alpha = phylanx::ir::node_data<double>{ 1e-5 };
    bool enable_output = true;

    // Time execution
    hpx::util::high_resolution_timer t;

    // Evaluate LRA using the read data
    auto result = lra(x, y, alpha);

    // CSV Header
    std::cout << "primitive_instance" << ","
        << "count" << ","
        << "time" << ","
        << "direct_count" << ","
        << "direct_time"
        << std::endl;

    // List of existing primitive instances
    std::vector<std::string> existing_primitive_instances;
    // Iterate over all primitive types
    for (auto const& pattern :
        phylanx::execution_tree::get_all_known_patterns())
    {
        // Name of the primitive
        std::string const& name = hpx::util::get<0>(pattern);

        // Retrieve all instances of this primitive type
        for (auto const& entry : hpx::agas::find_symbols(
                 hpx::launch::sync, "/phylanx/" + name + "#*"))
        {
            existing_primitive_instances.push_back(entry.first);
        }
    }

    // Print performance data
    for (auto const& entry :
        retrieve_counter_data(existing_primitive_instances, hpx::find_here(),
            std::vector<std::string>{"count/eval", "time/eval",
                "count/eval_direct", "time/eval_direct"}))
    {
        std::cout << "\"" << entry.first << "\"";
        for (auto const& counter_value : entry.second)
        {
            std::cout << "," << counter_value;
        }
        std::cout << std::endl;
    }

    auto elapsed = t.elapsed();

    // Make sure all counters are properly initialized, don't reset current counter values
    hpx::reinit_active_counters(false);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::init(argc, argv);
}
