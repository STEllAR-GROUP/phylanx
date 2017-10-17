//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_main.hpp>

#include <iostream>

///////////////////////////////////////////////////////////////////////////////
char const* const lra_code = R"(
    //
    // The logistic regression analysis algorithm assumes to be invoked with
    // the following variables predefined:
    //
    //       iterations: the number of iterations to be performed
    //       alpha:      the error residual
    //       x, y:       the data to analyze
    //
    block(
        store(weights, constant(0, x)),
        store(transx, transpose(x)),
        store(step, 0),
        while(
            step < iterations,
            block(
                store(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),
                store(error, pred - y),
                store(gradient, dot(transx, error)),
                store(weights, weights - (alpha * gradient)),
                store(step, step + 1)
            )
        ),
        weights
    )
)";

int main(int argc, char* argv[])
{
    Eigen::VectorXd v1(8, 1);
    v1 << 17.99, 20.57, 19.69, 11.42, 20.29, 12.45, 18.25, 13.71;

    Eigen::VectorXd v2(8, 1);
    v2 << 0, 0, 0, 0, 1, 1, 0, 0;

    phylanx::execution_tree::variables variables = {
        {"iterations", std::int64_t{750}},
        {"alpha", phylanx::ir::node_data<double>{1e-5}},
        {"x", phylanx::ir::node_data<double>(v1)},
        {"y", phylanx::ir::node_data<double>(v2)}
    };

    // generate an execution tree from the given code
    phylanx::execution_tree::primitive_argument_type p =
        phylanx::execution_tree::generate_tree(lra_code, variables);

    // evaluate generated execution tree
    hpx::future<phylanx::ir::node_data<double>> f =
        phylanx::execution_tree::numeric_operand(p);

    std::cout << "Result: \n" << f.get() << std::endl;

    return 0;
}

