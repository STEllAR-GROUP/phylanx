//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>
#include <utility>

void test_inversion_0d()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive inversion =
        phylanx::execution_tree::primitives::create_inverse_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        inversion.eval();

    HPX_TEST_EQ(
        0.2, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_inversion_0d_lit()
{
    phylanx::ir::node_data<double> lhs(5.0);

    phylanx::execution_tree::primitive inversion =
        phylanx::execution_tree::primitives::create_inverse_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        inversion.eval();

    HPX_TEST_EQ(
        0.2, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_inversion_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive inversion =
        phylanx::execution_tree::primitives::create_inverse_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        inversion.eval();

    blaze::DynamicMatrix<double> expected = blaze::inv(m);
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_inversion_0d();
    test_inversion_0d_lit();

    test_inversion_2d();

    return hpx::util::report_errors();
}


