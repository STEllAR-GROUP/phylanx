//  Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>
#include <utility>
#include <vector>

void hstack_operation_0d()
{
    phylanx::execution_tree::primitive first =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive second =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive hstack =
        hpx::new_<phylanx::execution_tree::primitives::hstack_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(first), std::move(second)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    blaze::DynamicVector<double> expected{42.0, 5.0};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void hstack_operation_1d()
{
    blaze::DynamicVector<double> v1{1, 2, 3, 4, 5};
    blaze::DynamicVector<double> v2{11, 12, 13, 14, 15, 16};

    phylanx::execution_tree::primitive first =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive second =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive hstack =
        hpx::new_<phylanx::execution_tree::primitives::hstack_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(first), std::move(second)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    blaze::DynamicVector<double> expected{
        1, 2, 3, 4, 5, 11, 12, 13, 14, 15, 16};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void hstack_operation_2d()
{
    blaze::DynamicMatrix<double> m1{{1, 2, 3},
                                    {4, 5, 6}};
    blaze::DynamicMatrix<double> m2{{11, 22},
                                    {12, 13}};

    phylanx::execution_tree::primitive first =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive second =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive hstack =
        hpx::new_<phylanx::execution_tree::primitives::hstack_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(first), std::move(second)});

    blaze::DynamicMatrix<double> expected{{1, 2, 3, 11, 22},
                                          {4, 5, 6, 12, 13}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        hstack.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    hstack_operation_0d();
    hstack_operation_1d();
    hstack_operation_2d();

    return hpx::util::report_errors();
}
