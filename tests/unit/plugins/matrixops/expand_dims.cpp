// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
void test_0d()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive expand_dims =
        phylanx::execution_tree::primitives::create_expand_dims(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        expand_dims.eval();

    blaze::DynamicVector<double> expected{ 42. };

    HPX_TEST_EQ(
        expected, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

///////////////////////////////////////////////////////////////////////////////
void test_1d()
{
    blaze::DynamicVector<double> subject{6., 9., 13., 42., 54.};
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive expand_dims =
        phylanx::execution_tree::primitives::create_expand_dims(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        expand_dims.eval();

    blaze::DynamicMatrix<double> expected{{6., 9., 13., 42., 54.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_1d_axis(std::int64_t axis)
{
    blaze::DynamicVector<double> subject{6., 9., 13., 42., 54.};
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive expand_dims =
        phylanx::execution_tree::primitives::create_expand_dims(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs),
                phylanx::execution_tree::primitive_argument_type{axis}
            }
        );

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        expand_dims.eval();

    if (axis == 0 || axis == -2)
    {
        blaze::DynamicMatrix<double> expected{{6., 9., 13., 42., 54.}};
        HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
            phylanx::execution_tree::extract_numeric_value(f.get()));
    }
    else
    {
        blaze::DynamicMatrix<double> expected{{6.}, {9.}, {13.}, {42.}, {54.}};
        HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
            phylanx::execution_tree::extract_numeric_value(f.get()));
    }
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
///////////////////////////////////////////////////////////////////////////////
void test_2d()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {13., 42.}};
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive expand_dims =
        phylanx::execution_tree::primitives::create_expand_dims(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        expand_dims.eval();

    blaze::DynamicTensor<double> expected{{{6., 9.}, {13., 42.}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_axis(std::int64_t axis)
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {13., 42.}};
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive expand_dims =
        phylanx::execution_tree::primitives::create_expand_dims(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs),
                phylanx::execution_tree::primitive_argument_type{axis}
            }
        );

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        expand_dims.eval();

    if (axis == 0 || axis == -3)
    {
        blaze::DynamicTensor<double> expected{{{6., 9.}, {13., 42.}}};

        HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
            phylanx::execution_tree::extract_numeric_value(f.get()));
    }
    else if (axis == 1 || axis == -2)
    {
        blaze::DynamicTensor<double> expected{{{6., 9.}}, {{13., 42.}}};

        HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
            phylanx::execution_tree::extract_numeric_value(f.get()));
    }
    else
    {
        blaze::DynamicTensor<double> expected{{{6.}, {9.}}, {{13.}, {42.}}};

        HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
            phylanx::execution_tree::extract_numeric_value(f.get()));
    }
}
#endif

int main(int argc, char* argv[])
{
    test_0d();

    test_1d();
    test_1d_axis(0);
    test_1d_axis(-2);

    test_1d_axis(1);
    test_1d_axis(-1);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_2d();

    test_2d_axis(0);
    test_2d_axis(-3);
    test_2d_axis(1);
    test_2d_axis(-2);
    test_2d_axis(2);
    test_2d_axis(-1);
#endif

    return hpx::util::report_errors();
}
