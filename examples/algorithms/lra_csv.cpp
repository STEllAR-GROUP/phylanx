//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <iostream>

#include <boost/program_options.hpp>

///////////////////////////////////////////////////////////////////////////////
char const* const lra_code = R"(block(
    //
    // Logistic regression analysis algorithm
    //
    //   x: [30, 2]
    //   y: [30]
    define(lra, alpha, iterations,enable_output,
        block(
            define(data,file_read_csv("/usr/lib/python3.5/site-packages/sklearn/datasets/data/breast_cancer.csv")),
            cout("data = ", data),
            define(x,slice(data,500,529,0,1)),
            cout("x = ",x),
            define(y,slice(data,500,529,30,30)),
            define(weights, constant(0.0, shape(x, 1))),                // weights: [2]
            define(transx, transpose(x)),                               // transx:  [2, 30]
            define(pred, constant(0.0, shape(x, 0))),
            define(error, constant(0.0, shape(x, 0))),
            define(gradient, constant(0.0, shape(x, 1))),
            define(step, 0),
            while(
                step < iterations,
                block(
                    if(enable_output, cout("step: ", step, ", ", weights)),
                    store(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),  // exp(-dot(x, weights)): [30], pred: [30]
                    store(error, pred - y),                            // error: [30]
                    store(gradient, dot(transx, error)),               // gradient: [2]
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
  // compile the given code
  phylanx::execution_tree::compiler::function_list snippets;
  auto lra = phylanx::execution_tree::compile(lra_code, snippets);

  // evaluate generated execution tree
  auto alpha = phylanx::ir::node_data<double>{1e-5};

  auto iterations = vm["num_iterations"].as<std::int64_t>();
  bool enable_output = vm.count("enable_output") != 0;

  auto result = lra( alpha, iterations, enable_output);

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
      ("enable_output,e", "enable progress output (default: false)")
      ("num_iterations,n",
       boost::program_options::value<std::int64_t>()->default_value(750),
       "number of iterations (default: 750)")
      ;

  return hpx::init(desc, argc, argv);
}
