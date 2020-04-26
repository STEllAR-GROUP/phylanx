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
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
void test_kmeans()
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

int main(int argc, char* argv[])
{
    test_kmeans();
    return hpx::util::report_errors();
}
