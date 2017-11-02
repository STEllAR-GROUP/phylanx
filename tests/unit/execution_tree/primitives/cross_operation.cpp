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

void test_cross_operation_1d()
{
    blaze::DynamicVector<double, blaze::rowVector> v1{17.99, 20.57, 19.69};
    blaze::DynamicVector<double, blaze::rowVector> v2{11.42, 20.29, 12.45};

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive cross =
        hpx::new_<phylanx::execution_tree::primitives::cross_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        cross.eval();
    blaze::DynamicMatrix<double, blaze::rowVector> expected(1UL, 3UL);
    blaze::row(expected, 0UL) = blaze::cross(v1, v2);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_cross_operation_1d();

    return hpx::util::report_errors();
}

