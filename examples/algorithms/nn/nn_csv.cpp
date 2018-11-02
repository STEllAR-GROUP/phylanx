//   Copyright (c) 2017 Weile Wei
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
#include <boost/program_options.hpp>

char const* const read_x_code = R"(block(
    //
    // Read X-data from given CSV file
    //
    define(read_x, filepath, row_start, row_stop, col_start, col_stop,
        slice(file_read_csv(filepath),
              list(row_start , row_stop),
              list(col_start , col_stop))
    ),
    read_x
))";

char const* const read_y_code = R"(block(
    //
    // Read Y-data from given CSV file
    //
    define(read_y, filepath, row_start, row_stop, col_stop,
        slice(file_read_csv(filepath),
              list(row_start , row_stop), col_stop)
    ),
    read_y
))";

///////////////////////////////////////////////////////////////////////////////
char const* const nn_code = R"(block(
    //
    // Neural Network algorithm
    //
    //   x: [N, M]
    //   y: [N]
    //
    define(nn, X, y, num_iter, lr,
        block(
            set_seed(0),
            define(inputlayer_neurons, slice(shape(X), 1)),
            define(hidden_layer_neurons, inputlayer_neurons/2),
            define(output_neurons, slice(shape(y), 0)),
            define(wh, random(list(inputlayer_neurons, hidden_layer_neurons), "uniform")),
            define(bh, random(list(1, hidden_layer_neurons), "uniform")),
            define(wout, random(list(hidden_layer_neurons, output_neurons), "uniform")),
            define(bout, random(list(1, output_neurons), "uniform")),
            for_each(lambda(i, block(
                define(hidden_layer_activations, __div(1, __add(1, exp(__minus(__add(dot(X, wh), bh)))))),
                define(output, __div(1, __add(1, exp(__minus(__add(dot(hidden_layer_activations, wout), bout)))))),
                define(Error, __sub(y, output)),
                define(d_output, __mul(Error, __mul(output, __sub(1, output)))),
                define(d_hidden_layer, __mul(dot(d_output, transpose(wout)), __mul(hidden_layer_activations, __sub(1, hidden_layer_activations)))),
                store(wout, __add(wout, __mul(dot(transpose(hidden_layer_activations), d_output), lr))),
                store(bout, __add(bout, __mul(sum(d_output, 0, true), lr))),
                store(wh, __add(wh, __mul(dot(transpose(X), d_hidden_layer), lr))),
                store(bh, __add(bh, __mul(sum(d_hidden_layer, 0, true), lr))))),
                range(num_iter)
            ), wh
        )
    ),
    nn
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

    auto lr = vm["lr"].as<double>();

    auto num_iter = vm["num_iter"].as<std::int64_t>();

    // evaluate NN using the read data
    auto const& code_nn =
        phylanx::execution_tree::compile("nn", nn_code, snippets);
    auto nn = code_nn.run();

    // time the execution
    hpx::util::high_resolution_timer t;

    auto result = nn(std::move(x), std::move(y), num_iter, lr);

    auto elapsed = t.elapsed();

    std::cout << "time: " << elapsed << " seconds" << std::endl;
    /*<< "Result: \n"
              << phylanx::execution_tree::extract_numeric_value(result)
              << std::endl
              */

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    boost::program_options::options_description desc("usage: nn [options]");
    desc.add_options()("num_iter,n",
        boost::program_options::value<std::int64_t>()->default_value(750),
        "number of iterations (default: 250)")("lr,lr",
        boost::program_options::value<double>()->default_value(1e-5),
        "lr (default: 1e-5)")("data_csv",
        boost::program_options::value<std::string>(),
        "file name for reading data")("row_start",
        boost::program_options::value<std::int64_t>()->default_value(0),
        "row_start (default: 0)")("col_start",
        boost::program_options::value<std::int64_t>()->default_value(0),
        "col_start (default: 0)")("row_stop",
        boost::program_options::value<std::int64_t>()->default_value(569),
        "row_stop (default: 569)")("col_stop",
        boost::program_options::value<std::int64_t>()->default_value(30),
        "col_stop (default: 30)");

    return hpx::init(desc, argc, argv);
}
