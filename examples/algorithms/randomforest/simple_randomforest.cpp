//   Copyright (c) 2017 Hartmut Kaiser
//   Copyright (c) 2018 Christopher Taylor
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <utility>
#include <cmath>
#include <iostream>

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
            define(model, randomforest_fit(train_data
                , train_labels
                , mx_depth
                , min_size
                , sample_size
                , n_trees)),
            block(
                store(test_labels, randomforest_predict(model, test_data)),
                cout(test_labels)
            )
        ),
        test_labels
    ),
    randomforest_test
))";

void test_randomforest()
{
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

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(randomforest_test_code, snippets);
    auto randomforest_test = code.run();

    // evaluate generated execution tree
    auto x = phylanx::ir::node_data<double>{v1};
    auto y = phylanx::ir::node_data<double>{v2};
    auto z = phylanx::ir::node_data<double>{v1};

    auto result = phylanx::execution_tree::extract_numeric_value(
        randomforest_test(x, y, z));

    std::cout << result << std::endl;
}

int main(int argc, char* argv[])
{
  test_randomforest();
  return hpx::util::report_errors();
}

