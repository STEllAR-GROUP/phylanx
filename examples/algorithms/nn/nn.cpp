//   Copyright (c) 2018 Weile Wei
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <cstdint>
#include <iostream>

#include <boost/program_options.hpp>
#include <blaze/Math.h>


//////////////////////////////////////////////////////////////////////////////
char const* const nn_code = R"(block(
    //
    // neural network algorithm
    //
    // parameters:
    //
    //num_iter: Setting training iterations
    //lr: Setting learning rate
    //inputlayer_neurons: number of features in data set
    //hiddenlayer_neurons: number of hidden layers neurons
    //output_neurons: number of neurons at output layer
    //
    //
    //
    define(nn, X, y, hiddenlayer_neurons, output_neurons, lr, num_iter,
        block(
            // weight and bias initialization
            define(inputlayer_neurons, shape(X, 1)),
            set_seed(0),
            define(wh, random(make_list(inputlayer_neurons, hiddenlayer_neurons), "uniform")),
            define(bh, random(make_list(1, hiddenlayer_neurons), "uniform")),
            define(wout, random(make_list(hiddenlayer_neurons, output_neurons), "uniform")),
            define(bout, random(make_list(1, output_neurons), "uniform")),
            // create local variables
            define(wh_local, wh),
            define(bh_local, bh),
            define(wout_local, wout),
            define(bout_local, bout),
            define(output, 0),
            define(hidden_layer_input1, 0),
            define(hidden_layer_input, 0),
            define(hiddenlayer_activations, 0),
            define(output_layer_input1, 0),
            define(output_layer_input, 0),
            define(slope_hidden_layer, 0),
            define(slope_output_layer, 0),
            define(E, 0),
            define(d_output, 0),
            define(Error_at_hidden_layer, 0),
            define(d_hiddenlayer, 0),
            map(
                lambda(
                    i,
                    block(
                        cout(i),
                        // forward propagation
                        store(hidden_layer_input1,dot(X, wh_local)),
                        store(hidden_layer_input,(hidden_layer_input1 + bh_local)),
                        store(hiddenlayer_activations, (1/(1+exp(-(hidden_layer_input))))),
                        store(output_layer_input1,dot(hiddenlayer_activations, wout_local)),
                        store(output_layer_input,(output_layer_input1 + bout_local)),
                        store(output,(1/(1+exp(-(output_layer_input))))),

                        // backward propagation
                        store(E, (y - output)),
                        store(slope_output_layer,(output*(1 - output))),
                        store(slope_hidden_layer,(hiddenlayer_activations*(1 - hiddenlayer_activations))),
                        store(d_output,(E * slope_output_layer)),
                        store(Error_at_hidden_layer,dot(d_output,transpose(wout_local))),
                        store(d_hiddenlayer,(Error_at_hidden_layer * slope_hidden_layer)),
                        store(wout_local,wout_local+(dot(transpose(hiddenlayer_activations),d_output)*lr)),
                        store(bout_local,bout_local+(sum(d_output, 0, true)*lr)),
                        store(wh_local,wh_local+(dot(transpose(X), d_hiddenlayer)*lr)),
                        store(bh_local,bh_local+(sum(d_hiddenlayer, 0, true)*lr))
                    )
                ), range(num_iter)),
            output
        )
    ),
    nn
))";

int hpx_main(boost::program_options::variables_map& vm)
{
    blaze::DynamicMatrix<double> v1{
            {15.04, 16.74}, {13.82, 24.49}, {12.54, 16.32}, {23.09, 19.83},
            {9.268, 12.87}, {9.676, 13.14}, {12.22, 20.04}, {11.06, 17.12},
            {16.3, 15.7}, {15.46, 23.95}, {11.74, 14.69}, {14.81, 14.7},
            {13.4, 20.52}, {14.58, 13.66}, {15.05, 19.07}, {11.34, 18.61},
            {18.31, 20.58}, {19.89, 20.26}, {12.88, 18.22}, {12.75, 16.7},
            {9.295, 13.9}, {24.63, 21.6}, {11.26, 19.83}, {13.71, 18.68},
            {9.847, 15.68}, {8.571, 13.1}, {13.46, 18.75}, {12.34, 12.27},
            {13.94, 13.17}, {12.07, 13.44}}; // 1st training dataset v1

//    blaze::DynamicMatrix<double> v1 {{1, 0, 1, 0}, {1, 0, 1, 1}, {0, 1, 0, 1}}; // 2nd training dataset v1

    blaze::DynamicVector<double> v2{1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1,
                                    0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}; // 1st training dataset v2
//    blaze::DynamicVector<double> v2{{1}, {1}, {0}}; // 2nd training dataset v2

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto nn = phylanx::execution_tree::compile("nn", nn_code, snippets);

    // evaluate generated execution tree
    auto X = phylanx::ir::node_data<double>{v1};
    auto y = phylanx::ir::node_data<double>{v2};
    auto lr = phylanx::ir::node_data<double>{1e-1};

    auto hiddenlayer_neurons = vm["hiddenlayer_neurons"].as<std::int64_t>();
    auto output_neurons = vm["output_neurons"].as<std::int64_t>();
    auto num_iter = vm["num_iter"].as<std::int64_t>();

//    bool enable_output = vm.count("enable_output") != 0;

    // Measure execution time
    hpx::util::high_resolution_timer t;

    auto result = nn(X, y, hiddenlayer_neurons, output_neurons, lr, num_iter);

    auto elapsed = t.elapsed();

    std::cout << "Result: \n"
              << phylanx::execution_tree::extract_numeric_value(result)
//              << elapsed
              << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    boost::program_options::options_description desc("usage: nn [options]");
    desc.add_options()
//            ("enable_output,e", "enable progress output (default: false)")
            ("hiddenlayer_neurons,hiddenlayer_neurons",
             boost::program_options::value<std::int64_t>()->default_value(3),
             "number of neurons in hidden layer (default: 20)")
            ("output_neurons,output_neurons",
             boost::program_options::value<std::int64_t>()->default_value(30),
             "number of neurons in output layer (default: 30)")
            ("num_iter,n",
             boost::program_options::value<std::int64_t>()->default_value(5000),
             "number of iterations (default: 5000)")
            ;

    return hpx::init(desc, argc, argv);
}