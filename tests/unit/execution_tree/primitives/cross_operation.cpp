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

void test_vector_cross_product()
{
    blaze::DynamicVector<double, blaze::rowVector> v1{1.0, 2.0, 3.0};
    blaze::DynamicVector<double, blaze::rowVector> v2{4.0, 5.0, 6.0};

    blaze::DynamicMatrix<double> expected{{-3.0, 6.0, -3.0}};

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


    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_one_vector_with_dimension_2()
{
    blaze::DynamicVector<double, blaze::rowVector> v1{1.0, 2.0};
    blaze::DynamicVector<double, blaze::rowVector> v2{4.0, 5.0, 6.0};

    blaze::DynamicMatrix<double> expected{{12.0, -6.0, -3.0}};
}

void test_both_vectors_with_dimension_2()
{
    blaze::DynamicVector<double, blaze::rowVector> v1{1.0, 2.0};
    blaze::DynamicVector<double, blaze::rowVector> v2{4.0, 5.0};

    blaze::DynamicMatrix<double> expected{{-3.0, 0.0, 0.0}};
}

void test_multiple_vector_cross_products()
{
    blaze::DynamicMatrix<double> v1{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    blaze::DynamicMatrix<double> v2{{4.0, 5.0, 6.0}, {1.0, 2.0, 3.0}};

    blaze::DynamicMatrix<double> expected{{-3.0, 6.0, -3.0}, {3.0, -6.0, 3.0}};
}

int main(int argc, char* argv[])
{
    test_vector_cross_product();
    test_one_vector_with_dimension_2();
    test_both_vectors_with_dimension_2();
    //test_multiple_vector_cross_products();

    return hpx::util::report_errors();
}

