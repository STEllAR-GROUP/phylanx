//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <utility>
#include <vector>

void test_random_0d()
{
    phylanx::execution_tree::primitive dim =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.0));

    phylanx::execution_tree::primitive const_ =
        phylanx::execution_tree::primitives::create_random(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(dim)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        const_.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(result.num_dimensions(), 1);
    HPX_TEST_EQ(result.dimension(0), 1);
}

void test_random_1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive dim =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive const_ =
        phylanx::execution_tree::primitives::create_random(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(dim)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        const_.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(result.num_dimensions(), 1);
    HPX_TEST_EQ(result.dimension(0), 1007);
}

void test_random_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 105UL);

    phylanx::execution_tree::primitive dim =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive const_ =
        phylanx::execution_tree::primitives::create_random(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(dim)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        const_.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(result.num_dimensions(), 2);
    HPX_TEST_EQ(result.dimension(0), 101);
    HPX_TEST_EQ(result.dimension(1), 105);
}

int main(int argc, char* argv[])
{
    test_random_0d();
    test_random_1d();
    test_random_2d();

    return hpx::util::report_errors();
}

