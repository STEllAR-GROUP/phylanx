// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <algorithm>
#include <utility>
#include <vector>
#include <blaze/Math.h>

void test_shuffle_operation_1d()
{
    using op_type = blaze::DynamicVector<double>;

    blaze::Rand<op_type> gen{};
    op_type v1 = gen.generate(100ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::ir::node_data<double> v2_node =
        phylanx::execution_tree::extract_numeric_value(lhs.eval_direct());
    op_type v2 = v2_node.vector();

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_shuffle_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();
    auto actual = phylanx::execution_tree::extract_numeric_value(f.get());
    auto actual_val = actual.vector();

    // Is it the same l-value?
    HPX_TEST_EQ(actual_val, v2);

    // Has it changed?
    //HPX_TEST_NEQ(actual_val, v1);

    // Is it the same size
    //HPX_TEST_EQ(actual_val.size(), v1.size());

    //// Does it contain all the elements?
    //for (double const& i : v1)
    //{
    //    //HPX_TEST_NEQ(std::find(actual_val.begin(), actual_val.end(), i), -1);
    //}
}

void test_shuffle_operation_2d()
{
    using op_type = blaze::DynamicMatrix<double>;
    using phylanx::util::matrix_row_iterator;

    blaze::Rand<op_type> gen{};
    op_type m1 = gen.generate(20ul, 20ul);
    op_type m2 = m1;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_shuffle_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();
    auto actual = phylanx::execution_tree::extract_numeric_value(f.get());
    auto actual_val = actual.matrix();

    // Is it the same l-value?
    //HPX_TEST_EQ(actual_val, m1);

    // Has it changed?
    HPX_TEST_NEQ(actual_val, m2);

    // Is it the same size
    HPX_TEST_EQ(actual_val.rows(), m2.rows());
    HPX_TEST_EQ(actual_val.columns(), m2.columns());

    // Does it contain all the elements?
    auto m2_end = matrix_row_iterator<op_type>(m2, m2.rows());

    //// Does it contain all the elements?
    //for (auto it = matrix_row_iterator<op_type>(m2); it != m2_end; ++it)
    //{
    //    /*HPX_TEST_NEQ(
    //        std::find_if(
    //            matrix_row_iterator<decltype(actual_val)>(actual_val),
    //            matrix_row_iterator<decltype(actual_val)>(
    //                actual_val, actual_val.rows()),
    //            [&it](auto x) { return *x == *it; }),
    //        -1);*/
    //}
}

int main(int argc, char* argv[])
{
    test_shuffle_operation_1d();

    test_shuffle_operation_2d();

    return hpx::util::report_errors();
}
