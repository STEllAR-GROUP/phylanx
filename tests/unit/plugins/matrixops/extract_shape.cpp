//   Copyright (c) 2017-2018 Hartmut Kaiser
//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

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

void test_extract_shape_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 117UL);

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
    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(117), *it++);
    HPX_TEST(it == result.end());
}

void test_extract_shape_2d_axis(std::int64_t axis, std::int64_t expected)
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 117UL);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(axis));

    phylanx::execution_tree::primitive shape =
        phylanx::execution_tree::primitives::create_extract_shape(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1), std::move(arg2)});

    phylanx::execution_tree::primitive_argument_type f = shape.eval().get();

    auto result =
        phylanx::execution_tree::extract_scalar_integer_value_strict(f);
    HPX_TEST_EQ(expected, result);
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_extract_shape_3d()
{
    blaze::Rand<blaze::DynamicTensor<double>> gen{};
    blaze::DynamicTensor<double> m = gen.generate(11UL, 17UL, 13UL);

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
    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(11), *it++);
    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(17), *it++);
    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(13), *it++);
    HPX_TEST(it == result.end());
}

void test_extract_shape_3d_axis(std::int64_t axis, std::int64_t expected)
{
    blaze::Rand<blaze::DynamicTensor<double>> gen{};
    blaze::DynamicTensor<double> m = gen.generate(11UL, 17UL, 13UL);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(axis));

    phylanx::execution_tree::primitive shape =
        phylanx::execution_tree::primitives::create_extract_shape(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1), std::move(arg2)});

    phylanx::execution_tree::primitive_argument_type f = shape.eval().get();

    auto result =
        phylanx::execution_tree::extract_scalar_integer_value_strict(f);
    HPX_TEST_EQ(expected, result);
}
#endif

int main(int argc, char* argv[])
{
    test_extract_shape_0d();

    test_extract_shape_1d();

    test_extract_shape_2d();
    test_extract_shape_2d_axis(0, 101);
    test_extract_shape_2d_axis(1, 117);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_extract_shape_3d();
    test_extract_shape_3d_axis(0, 11);
    test_extract_shape_3d_axis(1, 17);
    test_extract_shape_3d_axis(2, 13);
#endif

    return hpx::util::report_errors();
}
