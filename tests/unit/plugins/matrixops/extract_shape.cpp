//   Copyright (c) 2017 Hartmut Kaiser
//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <utility>
#include <vector>

void test_extract_shape_0d()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive shape =
        phylanx::execution_tree::primitives::create_extract_shape(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = shape.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_list_value(f).size());
}

void test_extract_shape_1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive shape =
        phylanx::execution_tree::primitives::create_extract_shape(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = shape.eval().get();

    auto result = phylanx::execution_tree::extract_list_value(f);
    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(1007), *result.begin());
}

void test_extract_shape_2d_1()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive shape =
        phylanx::execution_tree::primitives::create_extract_shape(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = shape.eval().get();

    auto result = phylanx::execution_tree::extract_list_value(f);
    auto it = result.begin();
    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(101), *it++);
    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(101), *it);
}

void test_extract_shape_2d_2()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive shape =
        phylanx::execution_tree::primitives::create_extract_shape(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1), std::move(arg2)});

    phylanx::execution_tree::primitive_argument_type f = shape.eval().get();

    auto result = phylanx::execution_tree::extract_numeric_value(f);
    auto it = result.begin();
    HPX_TEST_EQ(101, *it++);
    HPX_TEST_EQ(101, *it);
}

int main(int argc, char* argv[])
{
    test_extract_shape_0d();

    test_extract_shape_1d();

    test_extract_shape_2d_1();
    test_extract_shape_2d_2();

    return hpx::util::report_errors();
}
