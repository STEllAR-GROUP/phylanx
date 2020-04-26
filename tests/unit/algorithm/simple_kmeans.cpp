// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
void test_kmeans_as_primitive()
{
    blaze::DynamicMatrix<double> points{{2., 1.}, {2.5, 1.}, {0., 1.},
        {2.25, 0.5}, {1.25, 0.}, {1.5, 2.75}, {0., 1.75}, {3., 1.}, {3., 2.75},
        {2.5, 1.5}, {13.75, 7.25}, {14.25, 2.25}, {9.5, 1.}, {10.75, 5.75},
        {11., 5.5}, {13., 2.25}, {8.25, 4.25}, {14., 2.75}, {13.5, 1.},
        {12.25, 1.5}, {3.5, 9.}, {1.75, 12.75}, {1.5, 11.25}, {-2.5, 13.75},
        {1.5, 13.5}, {2., 15.25}, {1.25, 15.}, {1.5, 11.25}, {0.25, 9.},
        {5., 16.}};

    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(points));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(5));
    phylanx::execution_tree::primitive arg3 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(0));
    phylanx::execution_tree::primitive arg4 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});

    blaze::DynamicMatrix<double> initial_centroids{
        {10.75, 5.75}, {3., 2.75}, {0.25, 9.}};

    phylanx::execution_tree::primitive arg5 =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>(initial_centroids));

    phylanx::execution_tree::primitive kmeans =
        phylanx::execution_tree::primitives::create_kmeans(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0),
                std::move(arg1), std::move(arg2), std::move(arg3),
                std::move(arg4), std::move(arg5)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        kmeans.eval();

    blaze::DynamicMatrix<double> expected{
        {12.025, 3.35}, {1.8, 1.325}, {1.575, 12.675}};

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST(
        allclose(phylanx::ir::node_data<double>(std::move(expected)), result));
}

///////////////////////////////////////////////////////////////////////////////
char const* const kmeans_test = R"(
    define(points, [[ 0.75,  0.25], [ 1.25,  3.  ], [ 2.75,  3.  ],
                    [ 1.  ,  0.25], [ 3.  ,  0.25], [ 1.5 ,  2.75],
                    [ 3.  ,  0.  ], [ 3.  ,  3.  ], [ 2.75,  2.25],
                    [ 2.5 ,  1.75], [11.  ,  4.5 ], [10.5 ,  3.  ],
                    [ 9.5 ,  5.  ], [10.  ,  3.5 ], [11.25,  6.25],
                    [ 9.25,  3.  ], [11.75,  0.75], [10.  ,  2.75],
                    [12.  ,  5.75], [15.25,  2.25], [-3.  , 14.75],
                    [ 4.75, 10.25], [-1.25, 13.25], [-0.25, 13.  ],
                    [ 0.75,  9.25], [-0.25,  9.25], [ 2.5 ,  8.75],
                    [-2.25, 10.25], [-2.25, 10.75], [ 2.5 , 11.75]])
    define(iterations, 5)
    define(initial_centroids, [[-3., 14.75], [1.5, 2.75], [3., 0.]])
    define(enable_output, 0)
    define(result_cpp, kmeans(points, 3, iterations, enable_output, nil,
        initial_centroids))
    define(kmeans_physl, points, iterations, initial_centroids, enable_output,
            block(
                define(closest_centroids, points, centroids, block(
                    define(points_x,
                        expand_dims(slice_column(points, 0), -1)
                    ),
                    define(points_y,
                        expand_dims(slice_column(points, 1), -1)
                    ),
                    define(centroids_x,
                        slice_column(centroids, 0)
                    ),
                    define(centroids_y,
                        slice_column(centroids, 1)
                    ),
                    argmin(sqrt(
                                power(points_x - centroids_x, 2
                                )+power(points_y - centroids_y, 2)
                        ), 1
                    )
                )),
                define(move_centroids, points, closest, centroids, block(
                    fmap(lambda(k, block(
                                define(x, closest == k),
                                define(count, sum(x * constant(1, shape(x, 0)))),
                                define(sum_, sum(points * expand_dims(x, -1), 0)),
                                sum_/count
                        )),
                        range(shape(centroids, 0))
                    )
                )),
                define(centroids, initial_centroids),
                for(define(i, 0), i < iterations, store(i, i + 1),
                    block(
                        define(closest, closest_centroids(points, centroids)),
                        if(enable_output, block(
                            cout("centroids in iteration ", i,": ", centroids)
                        )),
                        store(centroids,
                              apply(vstack,
                                    list(move_centroids(points,
                                            closest,
                                            centroids))))
                    )
                ), centroids
            )
        )
    define(result_physl, kmeans_physl(points, iterations, initial_centroids,
        enable_output))

    if(result_cpp == result_physl, true, false)
)";

///////////////////////////////////////////////////////////////////////////////
void test_kmeans_cpp_physl()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(kmeans_test, snippets);
    auto km = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(km()),
        phylanx::ir::node_data<uint8_t>{1});
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_kmeans_as_primitive();
    test_kmeans_cpp_physl();
    return hpx::util::report_errors();
}
