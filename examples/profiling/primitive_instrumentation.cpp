//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/agas.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <hpx/program_options.hpp>

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
            define(step, 2),
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

///////////////////////////////////////////////////////////////////////////////
int hpx_main(hpx::program_options::variables_map& vm)
{
    // Compile the given code
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& code = phylanx::execution_tree::compile(
        "fib", phylanx::ast::generate_ast(fib_code), snippets);

    // Enable collection of performance data for all existing primitives
    auto existing_primitive_instances = phylanx::util::enable_measurements();

    auto const fibonacci = code.run();

    // Fibonacci arguments
    auto num_iterations = vm["num_iterations"].as<std::int64_t>();

    // Evaluate Fibonacci using the read data
    fibonacci(num_iterations);

    // CSV Header
    std::cout << "primitive_instance,display_name,count,time,eval_directs\n";

    // Print performance data
    for (auto const& entry :
        phylanx::util::retrieve_counter_data(existing_primitive_instances))
    {
        std::cout << "\"" << entry.first << "\",\""
                  << phylanx::execution_tree::compiler::primitive_display_name(
                         entry.first)
                  << "\"";
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
    hpx::program_options::options_description desc(
        "usage: primitive_instrumentation [options]");
    desc.add_options()
        ("num_iterations,n",
            hpx::program_options::value<std::int64_t>()->default_value(10),
            "number of iterations (default: 10)");

    return hpx::init(desc, argc, argv);
}
