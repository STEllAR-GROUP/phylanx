// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Blaze.h>
#include <blaze_tensor/Blaze.h>

///////////////////////////////////////////////////////////////////////////////
using custom_vector_type = blaze::CustomVector<double, true, true>;
using custom_matrix_type = blaze::CustomMatrix<double, true, true>;
using custom_tensor_type = blaze::CustomTensor<double, true, true>;

///////////////////////////////////////////////////////////////////////////////
void test_generic_operation_0d(std::string const& func_name,
    bool func(double))
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.5));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation_bool(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    HPX_TEST_EQ(func(0.5),
        phylanx::execution_tree::extract_scalar_boolean_value(f.get()));
}

///////////////////////////////////////////////////////////////////////////////
void test_generic_operation_1d(std::string const& func_name,
    blaze::DynamicVector<std::uint8_t> func(custom_vector_type))
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> n = gen.generate(22UL);
    custom_vector_type m(n.data(), n.size(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation_bool(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicVector<std::uint8_t> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

///////////////////////////////////////////////////////////////////////////////
void test_generic_operation_2d(std::string const& func_name,
    blaze::DynamicMatrix<std::uint8_t> func(custom_matrix_type))
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> n = gen.generate(22UL, 22UL);
    custom_matrix_type m(n.data(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation_bool(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicMatrix<std::uint8_t> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

void test_generic_operation_3d(std::string const& func_name,
    blaze::DynamicTensor<std::uint8_t> func(custom_tensor_type))
{
    blaze::Rand<blaze::DynamicTensor<double>> gen{};
    blaze::DynamicTensor<double> n = gen.generate(22UL, 22UL, 22UL);
    custom_tensor_type m(
        n.data(), n.pages(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation_bool(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicTensor<std::uint8_t> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<std::uint8_t>(std::move(expected)),
        phylanx::execution_tree::extract_boolean_value_strict(f.get()));
}

///////////////////////////////////////////////////////////////////////////////
void test_0d_operations()
{
    test_generic_operation_0d(
        "isnan", [](double v) -> bool { return std::isnan(v); });
    test_generic_operation_0d(
        "isinf", [](double v) -> bool { return std::isinf(v); });
    test_generic_operation_0d(
        "isfinite", [](double v) -> bool { return std::isfinite(v); });
    test_generic_operation_0d("isneginf",
        [](double m) -> bool { return std::isinf(m) && std::signbit(m); });
    test_generic_operation_0d("isposinf",
        [](double m) -> bool { return std::isinf(m) && !std::signbit(m); });
}

void test_1d_operations()
{
    test_generic_operation_1d("isnan",
        [](custom_vector_type m) -> blaze::DynamicVector<std::uint8_t> {
            return blaze::map(
                m, [](double m) -> bool { return std::isnan(m); });
        });
    test_generic_operation_1d("isinf",
        [](custom_vector_type m) -> blaze::DynamicVector<std::uint8_t> {
            return blaze::map(
                m, [](double m) -> bool { return std::isinf(m); });
        });
    test_generic_operation_1d("isfinite",
        [](custom_vector_type m) -> blaze::DynamicVector<std::uint8_t> {
            return blaze::map(
                m, [](double m) -> bool { return std::isfinite(m); });
        });
    test_generic_operation_1d("isneginf",
        [](custom_vector_type m) -> blaze::DynamicVector<std::uint8_t> {
            return blaze::map(
                m, [](double m) { return std::isinf(m) && std::signbit(m); });
        });
    test_generic_operation_1d("isposinf",
        [](custom_vector_type m) -> blaze::DynamicVector<std::uint8_t> {
            return blaze::map(
                m, [](double m) { return std::isinf(m) && !std::signbit(m); });
        });
}

void test_2d_operations()
{
    test_generic_operation_2d("isnan",
        [](custom_matrix_type m) -> blaze::DynamicMatrix<std::uint8_t> {
            return blaze::map(
                m, [](double m) -> bool { return std::isnan(m); });
        });
    test_generic_operation_2d("isinf",
        [](custom_matrix_type m) -> blaze::DynamicMatrix<std::uint8_t> {
            return blaze::map(
                m, [](double m) -> bool { return std::isinf(m); });
        });
    test_generic_operation_2d("isfinite",
        [](custom_matrix_type m) -> blaze::DynamicMatrix<std::uint8_t> {
            return blaze::map(
                m, [](double m) -> bool { return std::isfinite(m); });
        });
    test_generic_operation_2d("isneginf",
        [](custom_matrix_type m) -> blaze::DynamicMatrix<std::uint8_t> {
            return blaze::map(
                m, [](double m) { return std::isinf(m) && std::signbit(m); });
        });
    test_generic_operation_2d("isposinf",
        [](custom_matrix_type m) -> blaze::DynamicMatrix<std::uint8_t> {
            return blaze::map(
                m, [](double m) { return std::isinf(m) && !std::signbit(m); });
        });
}

void test_3d_operations()
{
    test_generic_operation_3d("isnan",
        [](custom_tensor_type m) -> blaze::DynamicTensor<std::uint8_t> {
            return blaze::map(
                m, [](double m) -> bool { return std::isnan(m); });
        });
    test_generic_operation_3d("isinf",
        [](custom_tensor_type m) -> blaze::DynamicTensor<std::uint8_t> {
            return blaze::map(
                m, [](double m) -> bool { return std::isinf(m); });
        });
    test_generic_operation_3d("isfinite",
        [](custom_tensor_type m) -> blaze::DynamicTensor<std::uint8_t> {
            return blaze::map(
                m, [](double m) -> bool { return std::isfinite(m); });
        });
    test_generic_operation_3d("isneginf",
        [](custom_tensor_type m) -> blaze::DynamicTensor<std::uint8_t> {
            return blaze::map(
                m, [](double m) { return std::isinf(m) && std::signbit(m); });
        });
    test_generic_operation_3d("isposinf",
        [](custom_tensor_type m) -> blaze::DynamicTensor<std::uint8_t> {
            return blaze::map(
                m, [](double m) { return std::isinf(m) && !std::signbit(m); });
        });
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_0d_operations();
    test_1d_operations();
    test_2d_operations();
    test_3d_operations();

    return hpx::util::report_errors();
}
