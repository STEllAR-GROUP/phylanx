// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <utility>
#include <vector>

void test_eye_operation_N()
{
    phylanx::execution_tree::primitive N =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(4));

    phylanx::execution_tree::primitive eye =
        phylanx::execution_tree::primitives::create_eye_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(N)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        eye.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(blaze::IdentityMatrix<double>(4ul)),
        result);
}

void test_eye_operation_NM()
{
    phylanx::execution_tree::primitive N =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(5));

    phylanx::execution_tree::primitive M =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));

    phylanx::execution_tree::primitive eye =
        phylanx::execution_tree::primitives::create_eye_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(N), std::move(M)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        eye.eval();

    blaze::DynamicMatrix<double> expected{{1., 0.},
                                          {0., 1.},
                                          {0., 0.},
                                          {0., 0.},
                                          {0., 0.}};
    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)), result);
}


void test_eye_operation_NMK_negative_K()
{
    phylanx::execution_tree::primitive N =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(5));

    phylanx::execution_tree::primitive M =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-2));

    phylanx::execution_tree::primitive eye =
        phylanx::execution_tree::primitives::create_eye_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(N), std::move(M), std::move(k)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        eye.eval();

    blaze::DynamicMatrix<double> expected{{0., 0.},
                                          {0., 0.},
                                          {1., 0.},
                                          {0., 1.},
                                          {0., 0.}};
    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)), result);
}

void test_eye_operation_NMK_postive_K()
{
    phylanx::execution_tree::primitive N =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(5));

    phylanx::execution_tree::primitive M =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive eye =
        phylanx::execution_tree::primitives::create_eye_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(N), std::move(M), std::move(k)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        eye.eval();

    blaze::DynamicMatrix<double> expected{{0., 1.},
                                          {0., 0.},
                                          {0., 0.},
                                          {0., 0.},
                                          {0., 0.}};
    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)), result);
}

void test_eye_operation_NMK_zero_K()
{
    phylanx::execution_tree::primitive N =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(3));

    phylanx::execution_tree::primitive M =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(3));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));

    phylanx::execution_tree::primitive eye =
        phylanx::execution_tree::primitives::create_eye_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(N), std::move(M), std::move(k) });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        eye.eval();

    blaze::DynamicMatrix<double> expected{{1., 0., 0.},
                                          {0., 1., 0.},
                                          {0., 0., 1.}};
    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)), result);
}

void test_eye_operation_NMK_all_zeros()
{
    phylanx::execution_tree::primitive N =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(3));

    phylanx::execution_tree::primitive M =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(3));

    phylanx::execution_tree::primitive k =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(5));

    phylanx::execution_tree::primitive eye =
        phylanx::execution_tree::primitives::create_eye_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(N), std::move(M), std::move(k) });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        eye.eval();

    blaze::DynamicMatrix<double> expected{{0., 0., 0.},
                                          {0., 0., 0.},
                                          {0., 0., 0.}};
    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)), result);
}
int main(int argc, char* argv[])
{
    test_eye_operation_N();
    test_eye_operation_NM();
    test_eye_operation_NMK_negative_K();
    test_eye_operation_NMK_postive_K();
    test_eye_operation_NMK_zero_K();
    test_eye_operation_NMK_all_zeros();
    return hpx::util::report_errors();
}

