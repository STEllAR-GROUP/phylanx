//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0.0. (See accompanying
//   file LICENSE_1_0.0.txt or copy at http://www.boost.org/LICENSE_1_0.0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <iostream>

#include <blaze/Math.h>
#include <boost/program_options.hpp>

#include <hpx/include/agas.hpp>
#include <hpx/runtime_fwd.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
char const* const als_code = R"(
    //
    // Alternating Least squares algorithm
    //
    //
    //
    define(als, ratings, regularization, num_factors, iterations, alpha, enable_output,
        block(
            define(num_users, shape(ratings, 0)),
            define(num_items, shape(ratings, 1)),
            define(conf, alpha * ratings),
            define(conf_u, constant(0.0, make_list(num_items))),
            define(conf_i, constant(0.0,make_list(num_users))),

            define(c_u, constant(0.0, make_list(num_items, num_items))),
            define(c_i, constant(0.0, make_list(num_users, num_users))),
            define(p_u, constant(0.0, make_list(num_items))),
            define(p_i, constant(0.0, make_list(num_users))),

            set_seed(0),
            define(X, random(make_list(num_users, num_factors))),
            define(Y, random(make_list(num_items, num_factors))),

            define(I_f, identity(num_factors)),
            define(I_i, identity(num_items)),
            define(I_u, identity(num_users)),
            define(k, 0),
            define(i, 0),
            define(u, 0),

            define(XtX, constant(0.0, make_list(num_factors, num_factors))),
            define(YtY, constant(0.0, make_list(num_factors, num_factors))),
            define(A, constant(0.0, make_list(num_factors, num_factors))),
            define(b, constant(0.0, make_list(num_factors))),

            while(k < iterations,
                block(
                    if(enable_output,
                        block(
                                cout("iteration ",k),
                                cout("X: ",X),
                                cout("Y: ",Y)
                        )
                    ),
                    store(YtY, dot(transpose(Y), Y) + regularization*I_f),
                    store(XtX, dot(transpose(X), X) + regularization*I_f),

                    while(u < num_users,
                        block(
                            store(conf_u, slice_row(conf, u)),
                            store(c_u, diag(conf_u)),
                            store(p_u, __ne(conf_u,0.0,true)),
                            store(A, dot(dot(transpose(Y),c_u),Y)+ YtY),
                            store(b, dot(dot(transpose(Y),(c_u + I_i)),transpose(p_u))),
                            set_row(X,u,u+1,1,dot(inverse(A),b)),
                            store(u, u+1)
                        )
                    ),
                    store(u, 0),
                    while(i < num_items,
                        block(
                            store(conf_i, slice_column(conf, i)),
                            store(c_i, diag(conf_i)),
                            store(p_i, __ne(conf_i,0.0,true)),
                            store(A, dot(dot(transpose(X),c_i),X) + XtX),
                            store(b, dot(dot(transpose(X),(c_i + I_u)),transpose(p_i))),
                            set_row(Y,i,i+1,1,dot(inverse(A),b)),
                            store(i, i+1)
                        )
                    ),
                    store(i, 0),
                    store(k,k+1)
                )
            ),
            make_list(X, Y)
        )
    )
    als
)";

int hpx_main(boost::program_options::variables_map& vm)
{
    blaze::DynamicMatrix<double> R{{0.0, 4.0, 0.0, 0.0, 0.0},
        {1.0, 0.0, 4.0, 0.0, 5.0}, {0.0, 0.0, 0.0, 2.0, 0.0},
        {0.0, 8.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 4.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 2.0},
        {1.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 5.0, 0.0},
        {1.0, 0.0, 0.0, 2.0, 0.0}};

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compile(als_code, snippets);
    auto als = snippets.run();

    // evaluate generated execution tree
    auto regularization = vm["regularization"].as<double>();
    auto iterations = vm["iterations"].as<int64_t>();
    auto num_factors = vm["factors"].as<int64_t>();
    auto alpha = vm["alpha"].as<double>();
    auto enable_output = vm.count("enable_output") != 0;

    // Measure execution time
    hpx::util::high_resolution_timer t;

    auto result = phylanx::execution_tree::extract_list_value(
        als(R, regularization, num_factors, iterations, alpha, enable_output));

    auto elapsed = t.elapsed();

    auto it = result.begin();
    // Make sure all counters are properly initialized, don't reset current
    // counter values
    hpx::reinit_active_counters(false);

    std::cout << "X: \n"
              << phylanx::execution_tree::extract_numeric_value(*it++)
              << "\nY: \n"
              << phylanx::execution_tree::extract_numeric_value(*it)
              << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    boost::program_options::options_description desc("usage: als [options]");
    desc.add_options()("enable_output,e",
        "enable progress output (default: false)")("iterations,i",
        boost::program_options::value<std::int64_t>()->default_value(10),
        "number of iterations (default: 10.0)")("factors,f",
        boost::program_options::value<std::int64_t>()->default_value(3),
        "number of factors (default: 10)")("alpha,a",
        boost::program_options::value<double>()->default_value(40),
        "alpha (default: 40)")("regularization,r",
        boost::program_options::value<double>()->default_value(0.1),
        "regularization (default: 0.1)");

    return hpx::init(desc, argc, argv);
}
