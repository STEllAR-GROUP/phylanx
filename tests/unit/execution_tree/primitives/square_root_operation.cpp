//   Copyright (c) 2017 Parsa Amini
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>
#include <utility>
#include <vector>

#include <blaze/Math.h>

void test_square_root_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive add =
        hpx::new_<phylanx::execution_tree::primitives::square_root_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
        std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f = add.eval();

    HPX_TEST_EQ(
        2.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_square_root_operation_1d()
{
    blaze::DynamicVector<double> v1{
        17.99, 20.57, 19.69, 11.42, 20.29, 12.45, 18.25, 13.71};

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive square_root =
        hpx::new_<phylanx::execution_tree::primitives::square_root_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        square_root.eval();
    blaze::DynamicVector<double> expected = blaze::sqrt(v1);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_square_root_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive square_root =
        hpx::new_<phylanx::execution_tree::primitives::square_root_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        square_root.eval();

    blaze::DynamicMatrix<double> expected = blaze::sqrt(m1);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_square_root_operation_0d();
    test_square_root_operation_1d();
    test_square_root_operation_2d();

    return hpx::util::report_errors();
}

