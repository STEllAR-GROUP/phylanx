//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <vector>

void test_unary_not_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), true);

    phylanx::execution_tree::primitive unary_minus =
        hpx::new_<phylanx::execution_tree::primitives::unary_not_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        unary_minus.eval();

    HPX_TEST_EQ(
        false, phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

void test_unary_not_operation_0d_lit()
{
    phylanx::execution_tree::primitive unary_minus =
        hpx::new_<phylanx::execution_tree::primitives::unary_not_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                false
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        unary_minus.eval();

    HPX_TEST_EQ(
        true, phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

void test_unary_not_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive unary_not =
        hpx::new_<phylanx::execution_tree::primitives::unary_not_operation>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(lhs)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        unary_not.eval();

    HPX_TEST_EQ(
        m.nonZeros() > 0,
        phylanx::execution_tree::extract_boolean_value(f.get()) != 0);
}

int main(int argc, char* argv[])
{
    //test_unary_not_operation_0d();
    //test_unary_not_operation_0d_lit();

    test_unary_not_operation_2d();

    return hpx::util::report_errors();
}


