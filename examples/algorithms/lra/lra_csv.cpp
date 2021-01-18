//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

#include <blaze/Math.h>
#include <hpx/program_options.hpp>

//////////////////////////////////////////////////////////////////////////////////
// This example uses part of the breast cancer dataset from UCI Machine Learning
// Repository.
//     https://archive.ics.uci.edu/ml/datasets/Breast+Cancer+Wisconsin+(Diagnostic)
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
    define(read_x, filepath, row_start, row_stop, col_start, col_stop,
        slice(file_read_csv(filepath),
              make_list(row_start , row_stop),
              make_list(col_start , col_stop))
    ),
    read_x
))";

char const* const read_y_code = R"(block(
    //
    // Read Y-data from given CSV file
    //
    define(read_y, filepath, row_start, row_stop, col_stop,
        slice(file_read_csv(filepath), make_list(row_start , row_stop), col_stop)
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
    define(__lra, x, y, alpha, iterations, enable_output,
        block(
            define(weights, constant(0.0, shape(x, 1))),            // weights: [M]
            define(transx, transpose(x)),                           // transx:  [M, N]
            define(pred, constant(0.0, shape(x, 0))),
            define(error, constant(0.0, shape(x, 0))),
            define(grad, constant(0.0, shape(x, 1))),
            define(step, 0),
            while(
                step < iterations,
                block(
                    if(enable_output, cout("step: ", step, ", ", weights)),
                    // exp(-dot(x, weights)): [N], pred: [N]
                    store(pred, sigmoid(dot(x, weights))),
                    store(error, pred - y),                         // error: [N]
                    store(grad, dot(transx, error)),            // grad: [M]
                    parallel_block(
                        store(weights, weights - (alpha * grad)),
                        store(step, step + 1)
                    )
                )
            ),
            weights
        )
    ),
    __lra
))";

int hpx_main(hpx::program_options::variables_map& vm)
{
    if (vm.count("data_csv") == 0)
    {
        std::cerr << "Please specify '--data_csv=data-file'";
        return hpx::finalize();
    }

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code_read_x =
        phylanx::execution_tree::compile("read_x", read_x_code, snippets);
    auto read_x = code_read_x.run();

    auto const& code_read_y =
        phylanx::execution_tree::compile("read_y", read_y_code, snippets);
    auto read_y = code_read_y.run();

    auto row_start = vm["row_start"].as<std::int64_t>();
    auto col_start = vm["col_start"].as<std::int64_t>();
    auto row_stop = vm["row_stop"].as<std::int64_t>();
    auto col_stop = vm["col_stop"].as<std::int64_t>();

    // read the data from the files
    auto x = read_x(vm["data_csv"].as<std::string>(), row_start, row_stop,
        col_start, col_stop);

    // col_start and col_stop omitted in this case as we know the last column
    // in our csv file
    // has the y values.
    auto y =
        read_y(vm["data_csv"].as<std::string>(), row_start, row_stop, col_stop);

    auto alpha = vm["alpha"].as<double>();

    auto iterations = vm["num_iterations"].as<std::int64_t>();
    bool enable_output = vm.count("enable_output") != 0;

    // evaluate LRA using the read data
    auto const& code_lra =
        phylanx::execution_tree::compile("lra", lra_code, snippets);
    auto lra = code_lra.run();

    // time the execution
    hpx::chrono::high_resolution_timer t;

    auto result =
        lra(std::move(x), std::move(y), alpha, iterations, enable_output);

    auto elapsed = t.elapsed();

    std::cout << "Result: \n"
              << phylanx::execution_tree::extract_numeric_value(result)
              << std::endl
              << "Calculated in: " << elapsed << " seconds" << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    hpx::program_options::options_description desc("usage: lra [options]");
    desc.add_options()("enable_output,e",
        "enable progress output (default: false)")("num_iterations,n",
        hpx::program_options::value<std::int64_t>()->default_value(750),
        "number of iterations (default: 750)")("alpha,a",
        hpx::program_options::value<double>()->default_value(1e-5),
        "alpha (default: 1e-5)")("data_csv",
        hpx::program_options::value<std::string>(),
        "file name for reading data")("row_start",
        hpx::program_options::value<std::int64_t>()->default_value(0),
        "row_start (default: 0)")("col_start",
        hpx::program_options::value<std::int64_t>()->default_value(0),
        "col_start (default: 0)")("row_stop",
        hpx::program_options::value<std::int64_t>()->default_value(569),
        "row_stop (default: 569)")("col_stop",
        hpx::program_options::value<std::int64_t>()->default_value(30),
        "col_stop (default: 30)");

    hpx::init_params params;
    params.desc_cmdline = desc;
    return hpx::init(argc, argv, params);
}
