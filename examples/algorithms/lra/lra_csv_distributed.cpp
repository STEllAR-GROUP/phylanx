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
#include <vector>

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
        annotate_d(
            slice(file_read_csv(filepath),
                list(row_start, row_stop), list(col_start, col_stop)),
            "read_x",
            list("tile",
                list("rows", row_start, row_stop),
                list("columns", col_start, col_stop)
            )
        )
    ),
    read_x
))";

char const* const read_y_code = R"(block(
    //
    // Read Y-data from given CSV file
    //
    define(read_y, filepath, row_start, row_stop, col_stop,
        annotate_d(
            slice(file_read_csv(filepath),
                list(row_start, row_stop), col_stop),
            "read_y",
            list("tile",
                list("rows", row_start, row_stop),
                list("columns", col_stop, col_stop+1)
            )
        )
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
            define(transx, transpose_d(x)),                     // transx:   [M, N]
            define(error, constant(0.0, shape(x, 0))),          // error:    [N]
            define(weights, constant(0.0, shape(x, 1))),        // weights:  [M]
            define(pred, constant(0.0, shape(x, 0))),           // pred:     [N]
            define(gradient, constant(0.0, shape(x, 1))),       // gradient: [M]
            define(step, 0),
            while(
                step < iterations,
                block(
                    if(enable_output, cout("step: ", step, ", ", weights)),
                    store(pred, sigmoid(dot_d(x, weights))),    // pred:     [N]
                    store(error, pred - y),                     // error:    [N]
                    store(gradient, dot_d(transx, error)),      // gradient: [M]
                    parallel_block(
                        store(weights, weights - (alpha * gradient)),
                        store(step, step + 1)
                    )
                )
            ),
            weights
        )
    ),
    __lra
))";

////////////////////////////////////////////////////////////////////////////////
void calculate_horizontal_tiling_parameters(std::int64_t& row_start,
    std::int64_t& row_stop)
{
    std::uint32_t num_localities = hpx::get_num_localities(hpx::launch::sync);
    std::uint32_t this_locality = hpx::get_locality_id();

    std::int64_t rows = row_stop - row_start;

    if (rows > num_localities)
    {
        rows = (rows + num_localities) / num_localities;
        row_start += this_locality * rows;
        row_stop = (std::min)(row_stop, row_start + rows);
    }
}

void calculate_vertical_tiling_parameters(
    std::int64_t& col_start, std::int64_t& col_stop)
{
    std::uint32_t num_localities = hpx::get_num_localities(hpx::launch::sync);
    std::uint32_t this_locality = hpx::get_locality_id();

    std::int64_t columns = col_stop - col_start;

    if (columns > num_localities)
    {
        columns = (columns + num_localities) / num_localities - 1;
        col_start = col_start + this_locality * columns;
        col_stop = col_start + columns;
    }
}

////////////////////////////////////////////////////////////////////////////////
int hpx_main(hpx::program_options::variables_map& vm)
{
    if (vm.count("data_csv") == 0)
    {
        std::cerr << "Please specify '--data_csv=data-file'";
        return hpx::finalize();
    }

    // compile the given code
    using namespace phylanx::execution_tree;

    compiler::function_list snippets;
    auto const& code_read_x = compile("read_x", read_x_code, snippets);
    auto read_x = code_read_x.run();

    auto const& code_read_y = compile("read_y", read_y_code, snippets);
    auto read_y = code_read_y.run();

    // handle command line arguments
    auto filename = vm["data_csv"].as<std::string>();

    auto row_start = vm["row_start"].as<std::int64_t>();
    auto row_stop = vm["row_stop"].as<std::int64_t>();
    auto col_start = vm["col_start"].as<std::int64_t>();
    auto col_stop = vm["col_stop"].as<std::int64_t>();

    auto alpha = vm["alpha"].as<double>();

    auto iterations = vm["num_iterations"].as<std::int64_t>();
    bool enable_output = vm.count("enable_output") != 0;

    // calculate tiling parameters for this locality, read data
    primitive_argument_type x, y;

    if (vm["tiling"].as<std::string>() == "horizontal")
    {
        calculate_horizontal_tiling_parameters(row_start, row_stop);

        // read the X-data from the file
        x = read_x(filename, row_start, row_stop, col_start, col_stop);

        // Y-data: col_stop omitted is the column in our CSV file
        y = read_y(filename, row_start, row_stop, col_stop);
    }
    else
    {
        // do no tile Y-data, full data loaded on all localities

        // Y-data: col_stop omitted is the column in our CSV file
        y = read_y(filename, row_start, row_stop, col_stop);

        calculate_vertical_tiling_parameters(col_start, col_stop);

        // read the X-data from the file
        x = read_x(filename, row_start, row_stop, col_start, col_stop);
    }

    // evaluate LRA using the read data
    auto const& code_lra = compile("lra", lra_code, snippets);
    auto lra = code_lra.run();

    // time the execution
    hpx::chrono::high_resolution_timer t;

    auto result =
        lra(std::move(x), std::move(y), alpha, iterations, enable_output);

    auto elapsed = t.elapsed();

    std::cout << "Result: \n"
              << extract_numeric_value(result) << std::endl
              << "Calculated in: " << elapsed << " seconds" << std::endl;

    return hpx::finalize();
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    using hpx::program_options::options_description;
    using hpx::program_options::value;

    // command line handling
    options_description desc("usage: lra [options]");
    desc.add_options()
        ("enable_output,e",
          "enable progress output (default: false)")
        ("num_iterations,n", value<std::int64_t>()->default_value(750),
          "number of iterations (default: 750)")
        ("alpha,a", value<double>()->default_value(1e-5),
          "alpha (default: 1e-5)")
        ("data_csv", value<std::string>(), "file name for reading data")
        ("row_start", value<std::int64_t>()->default_value(0),
          "row_start (default: 0)")
        ("row_stop", value<std::int64_t>()->default_value(569),
          "row_stop (default: 569)")
        ("col_start", value<std::int64_t>()->default_value(0),
          "col_start (default: 0)")
        ("col_stop", value<std::int64_t>()->default_value(30),
          "col_stop (default: 30)")
        ("tiling", value<std::string>()->default_value("horizontal"),
          "tiling method ('horizontal' (default) or 'vertical')")
    ;

    // make sure hpx_main is run on all localities
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    return hpx::init(desc, argc, argv, cfg);
}
