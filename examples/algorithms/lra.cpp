//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>
#include <hpx/hpx_main.hpp>

#include <iostream>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
char const* const lra_code = R"(
    //
    // Logistic regression analysis algorithm
    //
    //   x: [30, 2]
    //   y: [30]
    define(lra, x, y, alpha, iterations,
        block(
            define(weights, constant(0, shape(x, 0))),                  // weights: [2]
            define(transx, transpose(x)),                               // transx:  [2, 30]
            define(step, 0),
            while(
                step < iterations,
                block(
                    define(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),  // exp(-dot(x, weights)): [30], pred: [30]
                    define(error, pred - y),                            // error: [30]
                    define(gradient, dot(transx, error)),               // gradient: [2]
                    store(weights, weights - (alpha * gradient)),
                    store(step, step + 1)
                )
            ),
            weights
        )
    )
)";

int main(int argc, char* argv[])
{
//     blaze::DynamicMatrix<double> v1{
//         {15.04, 16.74, 13.82, 24.49, 12.54, 16.32, 23.09, 19.83, 9.268, 12.87,
//             9.676, 13.14, 12.22, 20.04, 11.06, 17.12, 16.3, 15.7, 15.46, 23.95,
//             11.74, 14.69, 14.81, 14.7, 13.4, 20.52, 14.58, 13.66, 15.05, 19.07},
//         {11.34, 18.61, 18.31, 20.58, 19.89, 20.26, 12.88, 18.22, 12.75, 16.7,
//             9.295, 13.9, 24.63, 21.6, 11.26, 19.83, 13.71, 18.68, 9.847, 15.68,
//             8.571, 13.1, 13.46, 18.75, 12.34, 12.27, 13.94, 13.17, 12.07,
//             13.44}};

    blaze::DynamicMatrix<double> v1{{15.04, 16.74}, {13.82, 24.49},
        {12.54, 16.32}, {23.09, 19.83}, {9.268, 12.87}, {9.676, 13.14},
        {12.22, 20.04}, {11.06, 17.12}, {16.3, 15.7}, {15.46, 23.95},
        {11.74, 14.69}, {14.81, 14.7}, {13.4, 20.52}, {14.58, 13.66},
        {15.05, 19.07}, {11.34, 18.61}, {18.31, 20.58}, {19.89, 20.26},
        {12.88, 18.22}, {12.75, 16.7}, {9.295, 13.9}, {24.63, 21.6},
        {11.26, 19.83}, {13.71, 18.68}, {9.847, 15.68}, {8.571, 13.1},
        {13.46, 18.75}, {12.34, 12.27}, {13.94, 13.17}, {12.07, 13.44}};

    blaze::DynamicMatrix<double> v2{{1}, {0}, {1}, {0}, {1}, {1}, {1}, {1}, {1},
        {0}, {1}, {1}, {0}, {1}, {0}, {1}, {0}, {0}, {1}, {1}, {1}, {0}, {1},
        {1}, {1}, {1}, {1}, {1}, {1}, {1}};

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto lra = phylanx::execution_tree::compile(lra_code, snippets);

    // evaluate generated execution tree
    auto x = phylanx::ir::node_data<double>{v1};
    auto y = phylanx::ir::node_data<double>{v2};
    auto alpha = phylanx::ir::node_data<double>{1e-5};
    auto iterations = std::int64_t{750};

    auto result = lra(x, y, alpha, iterations);

    std::cout << "Result: \n"
              << phylanx::execution_tree::extract_numeric_value(result)
              << std::endl;

    return 0;
}
