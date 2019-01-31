//   Copyright (c) 2017 Hartmut Kaiser
//   Copyright (c) 2018 Christopher Taylor 
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
//#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <utility>
#include <cmath>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
char const* const randomforest_test_code = R"(block(
    //
    // randomforest algorithm
    //
    define(randomforest_test, train_data, train_labels, test_data,
        block(
            define(test_labels, constant(0.0, shape(test_data, 0))),
            define(mx_depth, 10),
            define(min_size, 1),
            define(sample_size, 1.0),
            define(n_trees, 10),
            define(model, randomforest_fit(train_data, train_labels, mx_depth, min_size, sample_size, n_trees)),
            block(
                store(test_labels, randomforest_predict(model, test_data)),
                cout(test_data)
            )
        )
    ),
    randomforest_test
))";

int hpx_main(boost::program_options::variables_map& vm)
{
/*
    blaze::DynamicMatrix<double> v1{
        {15.04, 16.74}, {13.82, 24.49}, {12.54, 16.32}, {23.09, 19.83},
        {9.268, 12.87}, {9.676, 13.14}, {12.22, 20.04}, {11.06, 17.12},
        {16.3, 15.7}, {15.46, 23.95}, {11.74, 14.69}, {14.81, 14.7},
        {13.4, 20.52}, {14.58, 13.66}, {15.05, 19.07}, {11.34, 18.61},
        {18.31, 20.58}, {19.89, 20.26}, {12.88, 18.22}, {12.75, 16.7},
        {9.295, 13.9}, {24.63, 21.6}, {11.26, 19.83}, {13.71, 18.68},
        {9.847, 15.68}, {8.571, 13.1}, {13.46, 18.75}, {12.34, 12.27},
        {13.94, 13.17}, {12.07, 13.44}};

    blaze::DynamicVector<double> v2{1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1,
                                    0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1};
*/
    blaze::DynamicMatrix<double> train{{0.0, 4.0, 0.0, 0.0, 0.0},
        {1.0, 0.0, 4.0, 0.0, 5.0}, {0.0, 0.0, 0.0, 2.0, 0.0},
        {0.0, 8.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 4.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 2.0},
        {1.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 5.0, 0.0},
        {1.0, 0.0, 0.0, 2.0, 0.0}};

    blaze::DynamicVector<double> labels { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0 };

    // evaluate generated execution tree
    auto ntrees = vm["trees"].as<std::uint64_t>();
    auto mnsize = vm["minsize"].as<std::uint64_t>();
    auto mxdepth = vm["maxdepth"].as<std::uint64_t>();
    auto samplesize = vm["samples"].as<double>();

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(randomforest_test_code, snippets);
    auto randomforest_test = code.run();

    // evaluate generated execution tree
    auto x = phylanx::ir::node_data<double>{train};
    auto y = phylanx::ir::node_data<double>{labels};
    auto z = phylanx::ir::node_data<double>{train};

    randomforest_test(x, y, z);
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
  boost::program_options::options_description desc("usage: simple_randomforest [options]");
  desc.add_options()("trees,t",
    boost::program_options::value<std::uint64_t>()->default_value(5),
    "number of trees (default: 5)")("samples,s",
    boost::program_options::value<double>()->default_value(1.0),
    "ratio of sample size (default: 1.0)")("minsize,m",
    boost::program_options::value<std::uint64_t>()->default_value(1),
    "min size (default: 1")("maxdepth,d",
    boost::program_options::value<std::uint64_t>()->default_value(10),
    "max depth (default: 10)");

  return hpx::init(desc, argc, argv);
}
