//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/agas.hpp>
#include <hpx/include/performance_counters.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

#include <blaze/Math.h>

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

int main()
{
    // Compile the given code
    phylanx::execution_tree::compiler::function_list snippets;

    auto const fibonacci = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(fib_code), snippets);

    // Evaluate Fibonacci using the read data
    auto const result = fibonacci();

    // List of existing primitive instances
    std::vector<std::string> existing_primitive_instances;

    // Retrieve all primitive instances
    for (auto const& entry :
        hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*#*"))
    {
        existing_primitive_instances.push_back(entry.first);
    }

    // TODO: Finish the test

    return hpx::util::report_errors();
}
