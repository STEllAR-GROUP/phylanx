//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <blaze/Math.h>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
char const* const als_test = R"(
    define(ratings,[[0.0,4.0,0.0,0.0,0.0],
                    [1.0,0.0,4.0,0.0,5.0],
                    [0.0,0.0,0.0,2.0,0.0],
                    [0.0,8.0,0.0,0.0,0.0],
                    [0.0,0.0,4.0,0.0,0.0],
                    [0.0,0.0,0.0,0.0,0.0],
                    [0.0,0.0,0.0,0.0,2.0],
                    [1.0,0.0,0.0,0.0,0.0],
                    [0.0,0.0,0.0,5.0,0.0],
                    [1.0,0.0,0.0,2.0,0.0]])
    define(regularization, 0.1)
    define(num_factors, 3)
    define(iterations, 10)
    define(alpha, 40)
    define(enable_output, 0)
    define(result_cpp, als(ratings, regularization, num_factors, iterations,
            alpha, enable_output))
    define(als_physl, ratings, regularization, num_factors, iterations, alpha,
        enable_output, random_input,
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

                define(X, [[0.302805, 1.12279, 0.0730414],
                                   [0.0708592, 1.52007, -1.42233],
                                   [-0.13309, -0.291394, -1.76165],
                                   [-0.17307, 1.36688, -0.0876731],
                                   [-0.358996, 1.12531, -1.3395],
                                   [1.22061, -0.123463, 0.428373],
                                   [-0.124051, 1.41438, 0.229887],
                                   [2.00816, 1.62716, 0.604894],
                                   [0.230434, 1.59456, -0.96898],
                                   [-0.0649103, -0.782776, 0.591243]]),
                define(Y, [[-0.345186, -0.444233, -0.442653],
                           [-0.881801, -1.32323, -0.540916],
                           [0.907346, -0.112799, 0.229098],
                           [0.81527, 0.477525, -1.02618],
                           [-0.731458, 1.2927, 0.989476]]),
                if(random_input, block(
                        set_seed(0),
                        store(X, random(make_list(num_users, num_factors))),
                        store(Y, random(make_list(num_items, num_factors)))
                        )
                ),
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
                                store(b, dot(dot(transpose(Y),(c_u + I_i)),
                                    transpose(p_u))),
                                store(slice(X,list(u,u+1,1),nil),dot(inverse(A),b)),
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
                                store(b, dot(dot(transpose(X),(c_i + I_u)),
                                    transpose(p_i))),
                                store(slice(Y,list(i,i+1,1),nil),dot(inverse(A),b)),
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
    define(result_physl, als_physl(ratings, regularization, num_factors, iterations,
        alpha, enable_output, 1))

    define(X_diff, sum(absolute(slice(result_physl, 0) - slice(result_cpp, 0))) /
            (num_factors * shape(ratings, 0)))
    define(Y_diff, sum(absolute(slice(result_physl, 1) - slice(result_cpp, 1))) /
            (num_factors * shape(ratings, 1)))
    define(physl_cpp_match, if((X_diff < 1e-5) && (Y_diff < 1e-5), true, false))


    define(result_expected,'([[-0.867185, 0.240093, -1.62405],
            [-0.257688, 0.735767, 0.968469], [0.0100641, 0.96555, -0.543022],
            [-0.868835, 0.24055, -1.62714], [0.22776, 0.157143, 1.77557],
            [0, 0, 0], [-0.413139, 0.190837, 0.647198], [-0.6112, 1.23145, -0.530929],
            [0.0100952, 0.968535, -0.544701], [-0.480196, 1.59023, -0.765669]],
            [[-0.759349, 0.580224, 0.367949], [-0.904612, -0.239779, -0.166589],
            [-0.774534, 0.233851, 0.641465], [1.15658, 0.804942, -0.37349],
            [-1.35317, 0.0204483, 0.654514]]))
    store(result_physl, als_physl(ratings, regularization, num_factors, iterations,
        alpha, enable_output, 0))
    store(X_diff, sum(absolute(slice(result_physl, 0) - slice(result_expected, 0))) /
            (num_factors * shape(ratings,0)))
    store(Y_diff, sum(absolute(slice(result_physl, 1) - slice(result_expected, 1))) /
            (num_factors * shape(ratings,1)))

    define(physl_expected_match, if((X_diff < 1e-5) && (Y_diff < 1e-5), true, false))

    if(!physl_cpp_match, cout("PHYSL result does not match the cpp result"))
    if(!physl_expected_match, cout("PHYSL result does not match the expected value"))

    if(physl_cpp_match && physl_expected_match, true, false)
)";

void test_als_physl()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(als_test, snippets);
    auto als = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(als),
        phylanx::ir::node_data<uint8_t>{1});
}

int main(int argc, char* argv[])
{
    test_als_physl();
    return hpx::util::report_errors();
}
