//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <utility>
#include <cmath>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
char const* const lra_code = R"(block(
    //
    // Logistic regression analysis algorithm
    //
    define(lra, x, y, alpha, iterations,
        block(
            define(weights, constant(0.0, shape(x, 1))),
            define(transx, transpose(x)),
            define(pred, constant(0.0, shape(x, 0))),
            define(error, constant(0.0, shape(x, 0))),
            define(gradient, constant(0.0, shape(x, 1))),
            define(step, 0),
            while(
                step < iterations,
                block(
                    store(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),
                    store(error, pred - y),
                    store(gradient, dot(transx, error)),
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

void test_lra()
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
    auto const& code = phylanx::execution_tree::compile(lra_code, snippets);
    auto lra = code.run();

    // evaluate generated execution tree
    auto x = phylanx::ir::node_data<double>{v1};
    auto y = phylanx::ir::node_data<double>{v2};
    auto alpha = phylanx::ir::node_data<double>{1e-5};
    auto iterations = phylanx::ir::node_data<double>{750};

    auto result = lra(x, y, alpha, iterations);

    // convert to integer
    blaze::DynamicVector<int> expected{-152777, 505757};
    blaze::DynamicVector<int> actual(2);

    actual[0] = std::ceil(
        phylanx::execution_tree::extract_numeric_value(result)[0] * 10000000);

    actual[1] = std::floor(
        phylanx::execution_tree::extract_numeric_value(result)[1] * 10000000);

    HPX_TEST_EQ(std::move(expected), std::move(actual));
}

int main(int argc, char* argv[])
{
  test_lra();
  return hpx::util::report_errors();
}

