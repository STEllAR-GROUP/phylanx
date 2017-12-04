//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <iostream>

#include <boost/program_options.hpp>
#include <blaze/Math.h>

//////////////////////////////////////////////////////////////////////////////////
// This example uses part of the breast cancer dataset from UCI Machine Learning
// Repository. https://goo.gl/U2Uwz
//
// A copy of the full dataset in CSV format (breast_cancer.csv), obtained from
// scikit-learn datasets, is provided in the same folder as this example.
//
// The layout of the data in the provided CSV file used by the example
// is as follows:
// 30 features per line followed by the classification
// 569 lines of data
/////////////////////////////////////////////////////////////////////////////////

char const* const read_x_code = R"(block(
    //
    // Read X-data from given CSV file
    //
    define(read_x, filepath,
        slice(file_read_csv(filepath), 500, 530, 0, 2)
    ),
    read_x
))";

char const* const read_y_code = R"(block(
    //
    // Read Y-data from given CSV file
    //
    define(read_y, filepath,
        slice(file_read_csv(filepath), 500, 530, 30, 31)
    ),
    read_y
))";

///////////////////////////////////////////////////////////////////////////////
char const* const lra_code = R"(block(
    //
    // Logistic regression analysis algorithm
    //
    //   x: [N, M]
    //   y: [N]
    //
    define(lra, x, y, alpha, iterations, enable_output,
        block(
            define(weights, constant(0.0, shape(x, 1))),                // weights: [M]
            define(transx, transpose(x)),                               // transx:  [M, N]
            define(pred, constant(0.0, shape(x, 0))),
            define(error, constant(0.0, shape(x, 0))),
            define(gradient, constant(0.0, shape(x, 1))),
            define(step, 0),
            while(
                step < iterations,
                block(
                    if(enable_output, cout("step: ", step, ", ", weights)),
                    store(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),  // exp(-dot(x, weights)): [N], pred: [N]
                    store(error, pred - y),                            // error: [N]
                    store(gradient, dot(transx, error)),               // gradient: [M]
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

int hpx_main(boost::program_options::variables_map& vm)
{
    if (vm.count("data_csv") == 0)
    {
        std::cerr << "Please specify '--data_csv=data-file'";
        return hpx::finalize();
    }

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto read_x = phylanx::execution_tree::compile(read_x_code, snippets);
    auto read_y = phylanx::execution_tree::compile(read_y_code, snippets);

    // read the data from the files
    auto x = read_x(vm["data_csv"].as<std::string>());
    auto y = read_y(vm["data_csv"].as<std::string>());

    auto alpha = vm["alpha"].as<double>();

    auto iterations = vm["num_iterations"].as<std::int64_t>();
    bool enable_output = vm.count("enable_output") != 0;

    // evaluate LRA using the read data
    auto lra = phylanx::execution_tree::compile(lra_code, snippets);
    auto result =
        lra(std::move(x), std::move(y), alpha, iterations, enable_output);

    std::cout << "Result: \n"
              << phylanx::execution_tree::extract_numeric_value(result)
              << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    boost::program_options::options_description desc("usage: lra [options]");
    desc.add_options()
        ("enable_output,e",
        "enable progress output (default: false)")
        ("num_iterations,n",
            boost::program_options::value<std::int64_t>()->default_value(750),
            "number of iterations (default: 750)")
        ("alpha,a",
            boost::program_options::value<double>()->default_value(1e-5),
            "alpha (default: 1e-5)")
        ("data_csv",
            boost::program_options::value<std::string>(),
            "file name for reading data")
        ;

    return hpx::init(desc, argc, argv);
}
