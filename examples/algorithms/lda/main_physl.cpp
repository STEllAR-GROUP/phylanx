//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <cstdint>
#include <iostream>

#include <hpx/program_options.hpp>
#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
char const* const lda_code = R"(block(
    //
    // Logistic regression analysis algorithm
    //
    //   x: [30, 2]
    //   t: 3
    //   alpha : 0.01
    //   beta : 0.1
    //
    define(lda, x, y, alpha, iterations, enable_output,
        block(
            define(weights, constant(0.0, shape(x, 1))),         // weights: [2]
            define(transx, transpose(x)),                        // transx:  [2, 30]
            define(pred, constant(0.0, shape(x, 0))),
            define(error, constant(0.0, shape(x, 0))),
            define(gradient, constant(0.0, shape(x, 1))),
            define(step, 0),
            while(
                step < iterations,
                block(
                    if(enable_output, cout("step: ", step, ", ", weights)),
                    // exp(-dot(x, weights)): [30], pred: [30]
                    store(pred, sigmoid(dot(x, weights))),
                    store(error, pred - y),                      // error: [30]
                    store(gradient, dot(transx, error)),         // gradient: [2]
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

int hpx_main(hpx::program_options::variables_map& vm)
{
    blaze::DynamicMatrix<double> v1{
        {15., 16.}, {13., 24.}, {12., 16.}, {23., 19.},
        {9., 12.}, {9., 13.}, {12., 20.}, {11., 17.},
        {16., 15.}, {15., 23.}, {11., 14.}, {14., 14.},
        {13., 20.}, {14., 13.}, {15., 19.}, {11., 18.},
        {18., 20.}, {19., 20.}, {12., 18.}, {12., 16.},
        {9., 13.}, {24., 21.}, {11., 19.}, {13., 18.},
        {9., 15.}, {8., 13.}, {13., 18.}, {12., 12.},
        {13., 13.}, {12., 13.}};

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code =
        phylanx::execution_tree::compile("lra", lra_code, snippets);
    auto lra = code.run();

    // evaluate generated execution tree
    auto x = phylanx::ir::node_data<double>{v1};
    auto y = phylanx::ir::node_data<double>{v2};
    auto alpha = phylanx::ir::node_data<double>{1e-5};

    auto iterations = vm["num_iterations"].as<std::int64_t>();
    bool enable_output = vm.count("enable_output") != 0;

    auto result = lra(x, y, alpha, iterations, enable_output);

    std::cout << "Result: \n"
              << phylanx::execution_tree::extract_numeric_value(result)
              << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    hpx::program_options::options_description desc("usage: lra [options]");
    desc.add_options()
        ("enable_output,e", "enable progress output (default: false)")
        ("num_iterations,n",
            hpx::program_options::value<std::int64_t>()->default_value(750),
            "number of iterations (default: 750)")
        ;

    return hpx::init(desc, argc, argv);
}

