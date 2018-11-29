// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Blaze.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Blaze.h>
#endif

///////////////////////////////////////////////////////////////////////////////
using custom_vector_type = blaze::CustomVector<double, true, true>;
using custom_matrix_type = blaze::CustomMatrix<double, true, true>;
using custom_tensor_type = blaze::CustomTensor<double, true, true>;

///////////////////////////////////////////////////////////////////////////////
void test_generic_operation_0d(std::string const& func_name,
    double func(double))
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.5));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    HPX_TEST_EQ(
        func(0.5), phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_generic_operation_0d_greater1(std::string const& func_name,
    double func(double))
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    HPX_TEST_EQ(
        func(5.0), phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

///////////////////////////////////////////////////////////////////////////////
void test_generic_operation_1d(std::string const& func_name,
    blaze::DynamicVector<double> func(custom_vector_type))
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> n = gen.generate(22UL);
    custom_vector_type m(n.data(), n.size(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicVector<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_generic_operation_1d(std::string const& func_name,
    double func(custom_vector_type))
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> n = gen.generate(22UL);
    custom_vector_type m(n.data(), n.size(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    double expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_generic_operation_1d_greater1(std::string const& func_name,
    blaze::DynamicVector<double> func(custom_vector_type))
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> n = gen.generate(22UL, 1, 5);
    custom_vector_type m(n.data(), n.size(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicVector<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

///////////////////////////////////////////////////////////////////////////////
void test_generic_operation_2d(std::string const& func_name,
    blaze::DynamicMatrix<double> func(custom_matrix_type))
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> n = gen.generate(22UL, 22UL);
    custom_matrix_type m(n.data(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicMatrix<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_generic_operation_2d(std::string const& func_name,
    double func(custom_matrix_type))
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> n = gen.generate(22UL, 22UL);
    custom_matrix_type m(n.data(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    double expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_generic_operation_2d_greater1(std::string const& func_name,
    blaze::DynamicMatrix<double>
        func(custom_matrix_type))
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> n = gen.generate(22UL, 22UL, 1, 5);
    custom_matrix_type m(n.data(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicMatrix<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_generic_operation_3d(std::string const& func_name,
    blaze::DynamicTensor<double> func(custom_tensor_type))
{
    blaze::Rand<blaze::DynamicTensor<double>> gen{};
    blaze::DynamicTensor<double> n = gen.generate(22UL, 22UL, 22UL);
    custom_tensor_type m(
        n.data(), n.pages(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicTensor<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_generic_operation_3d(std::string const& func_name,
    double func(custom_tensor_type))
{
    blaze::Rand<blaze::DynamicTensor<double>> gen{};
    blaze::DynamicTensor<double> n = gen.generate(22UL, 22UL, 22UL);
    custom_tensor_type m(
        n.data(), n.pages(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    double expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_generic_operation_3d_greater1(std::string const& func_name,
    blaze::DynamicTensor<double>
        func(custom_tensor_type))
{
    blaze::Rand<blaze::DynamicTensor<double>> gen{};
    blaze::DynamicTensor<double> n = gen.generate(22UL, 22UL, 22UL, 1, 5);
    custom_tensor_type m(
        n.data(), n.pages(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicTensor<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}
#endif

///////////////////////////////////////////////////////////////////////////////
void test_0d_operations()
{
    test_generic_operation_0d("amin", [](double m) -> double { return m; });
    test_generic_operation_0d("amax", [](double m) -> double { return m; });
    test_generic_operation_0d("absolute", std::abs);
    test_generic_operation_0d("floor", std::floor);
    test_generic_operation_0d("ceil", std::ceil);
    test_generic_operation_0d("trunc", std::trunc);
    test_generic_operation_0d("rint", std::round);
    test_generic_operation_0d("conj", blaze::conj);
    test_generic_operation_0d("real", std::real);
    test_generic_operation_0d("imag", std::imag);
    test_generic_operation_0d("sqrt", std::sqrt);
    test_generic_operation_0d("cbrt", std::cbrt);
    test_generic_operation_0d("exp", std::exp);
    test_generic_operation_0d("exp2", std::exp2);
    test_generic_operation_0d("log", std::log);
    test_generic_operation_0d("log2", std::log2);
    test_generic_operation_0d("log10", std::log10);
    test_generic_operation_0d("sin", std::sin);
    test_generic_operation_0d("cos", std::cos);
    test_generic_operation_0d("tan", std::tan);
    test_generic_operation_0d("arcsin", std::asin);
    test_generic_operation_0d("arccos", std::acos);
    test_generic_operation_0d("arctan", std::atan);
    test_generic_operation_0d("arcsinh", std::asinh);
    test_generic_operation_0d_greater1("arccosh", std::acosh);
    test_generic_operation_0d("arctanh", std::atanh);

    test_generic_operation_0d("invsqrt", blaze::invsqrt);
    test_generic_operation_0d("invcbrt", blaze::invcbrt);
    test_generic_operation_0d(
        "exp10", [](double m) -> double { return std::pow(10, m); });
    test_generic_operation_0d("erf", blaze::erf);
    test_generic_operation_0d("erfc", blaze::erfc);

    test_generic_operation_0d("trace", std::abs);
}

void test_1d_operations()
{
    test_generic_operation_1d(
        "amin", [](custom_vector_type m) -> double {
            return (blaze::min)(m);
        });
    test_generic_operation_1d(
        "amax", [](custom_vector_type m) -> double {
            return (blaze::max)(m);
        });
    test_generic_operation_1d(
        "absolute", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::abs(m);
        });
    test_generic_operation_1d(
        "floor", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::floor(m);
        });
    test_generic_operation_1d(
        "ceil", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::ceil(m);
        });
    test_generic_operation_1d(
        "trunc", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::trunc(m);
        });
    test_generic_operation_1d(
        "rint", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::round(m);
        });
    test_generic_operation_1d(
        "conj", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::conj(m);
        });
    test_generic_operation_1d(
        "real", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::real(m);
        });
    test_generic_operation_1d(
        "imag", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::imag(m);
        });
    test_generic_operation_1d(
        "sqrt", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::sqrt(m);
        });
    test_generic_operation_1d(
        "cbrt", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::cbrt(m);
        });
    test_generic_operation_1d(
        "exp", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::exp(m);
        });
    test_generic_operation_1d(
        "exp2", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::exp2(m);
        });
    test_generic_operation_1d(
        "log", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::log(m);
        });
    test_generic_operation_1d(
        "log2", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::log2(m);
        });
    test_generic_operation_1d(
        "log10", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::log10(m);
        });
    test_generic_operation_1d(
        "sin", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::sin(m);
        });
    test_generic_operation_1d(
        "cos", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::cos(m);
        });
    test_generic_operation_1d(
        "tan", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::tan(m);
        });
    test_generic_operation_1d(
        "arcsin", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::asin(m);
        });
    test_generic_operation_1d(
        "arccos", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::acos(m);
        });
    test_generic_operation_1d(
        "arctan", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::atan(m);
        });
    test_generic_operation_1d(
        "arcsinh", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::asinh(m);
        });
    test_generic_operation_1d_greater1(
        "arccosh", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::acosh(m);
        });
    test_generic_operation_1d(
        "arctanh", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::atanh(m);
        });

    test_generic_operation_1d(
        "invsqrt", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::invsqrt(m);
        });
    test_generic_operation_1d(
        "invcbrt", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::invcbrt(m);
        });
    test_generic_operation_1d(
        "exp10", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::exp10(m);
        });
    test_generic_operation_1d(
        "erf", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::erf(m);
        });
    test_generic_operation_1d(
        "erfc", [](custom_vector_type m) -> blaze::DynamicVector<double> {
            return blaze::erfc(m);
        });

//     test_generic_operation_1d(
//         "normalize", [](custom_vector_type m) -> blaze::DynamicVector<double> {
//             return blaze::normalize(m);
//         });
}

void test_2d_operations()
{
    test_generic_operation_2d(
        "amin", [](custom_matrix_type m) -> double {
            return (blaze::min)(m);
        });
    test_generic_operation_2d(
        "amax", [](custom_matrix_type m) -> double {
            return (blaze::max)(m);
        });
    test_generic_operation_2d(
        "absolute", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::abs(m);
        });
    test_generic_operation_2d(
        "floor", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::floor(m);
        });
    test_generic_operation_2d(
        "ceil", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::ceil(m);
        });
    test_generic_operation_2d(
        "trunc", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::trunc(m);
        });
    test_generic_operation_2d(
        "rint", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::round(m);
        });
    test_generic_operation_2d(
        "conj", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::conj(m);
        });
    test_generic_operation_2d(
        "real", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::real(m);
        });
    test_generic_operation_2d(
        "imag", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::imag(m);
        });
    test_generic_operation_2d(
        "sqrt", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::sqrt(m);
        });
    test_generic_operation_2d(
        "cbrt", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::cbrt(m);
        });
    test_generic_operation_2d(
        "exp", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::exp(m);
        });
    test_generic_operation_2d(
        "exp2", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::exp2(m);
        });
    test_generic_operation_2d(
        "log", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::log(m);
        });
    test_generic_operation_2d(
        "log2", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::log2(m);
        });
    test_generic_operation_2d(
        "log10", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::log10(m);
        });
    test_generic_operation_2d(
        "sin", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::sin(m);
        });
    test_generic_operation_2d(
        "cos", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::cos(m);
        });
    test_generic_operation_2d(
        "tan", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::tan(m);
        });
    test_generic_operation_2d(
        "arcsin", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::asin(m);
        });
    test_generic_operation_2d(
        "arccos", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::acos(m);
        });
    test_generic_operation_2d(
        "arctan", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::atan(m);
        });
    test_generic_operation_2d(
        "arcsinh", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::asinh(m);
        });
    test_generic_operation_2d_greater1(
        "arccosh", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::acosh(m);
        });
    test_generic_operation_2d(
        "arctanh", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::atanh(m);
        });

    test_generic_operation_2d(
        "invsqrt", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::invsqrt(m);
        });
    test_generic_operation_2d(
        "invcbrt", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::invcbrt(m);
        });
    test_generic_operation_2d(
        "exp10", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::exp10(m);
        });
    test_generic_operation_2d(
        "erf", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::erf(m);
        });
    test_generic_operation_2d(
        "erfc", [](custom_matrix_type m) -> blaze::DynamicMatrix<double> {
            return blaze::erfc(m);
        });

    test_generic_operation_2d(
        "trace", [](custom_matrix_type m) -> double {
            return blaze::trace(m);
        });
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_3d_operations()
{
    test_generic_operation_3d(
        "amin", [](custom_tensor_type m) -> double {
            return (blaze::min)(m);
        });
    test_generic_operation_3d(
        "amax", [](custom_tensor_type m) -> double {
            return (blaze::max)(m);
        });
    test_generic_operation_3d(
        "absolute", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::abs(m);
        });
    test_generic_operation_3d(
        "floor", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::floor(m);
        });
    test_generic_operation_3d(
        "ceil", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::ceil(m);
        });
    test_generic_operation_3d(
        "trunc", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::trunc(m);
        });
    test_generic_operation_3d(
        "rint", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::round(m);
        });
    test_generic_operation_3d(
        "conj", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::conj(m);
        });
    test_generic_operation_3d(
        "real", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::real(m);
        });
    test_generic_operation_3d(
        "imag", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::imag(m);
        });
    test_generic_operation_3d(
        "sqrt", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::sqrt(m);
        });
    test_generic_operation_3d(
        "cbrt", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::cbrt(m);
        });
    test_generic_operation_3d(
        "exp", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::exp(m);
        });
    test_generic_operation_3d(
        "exp2", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::exp2(m);
        });
    test_generic_operation_3d(
        "log", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::log(m);
        });
    test_generic_operation_3d(
        "log2", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::log2(m);
        });
    test_generic_operation_3d(
        "log10", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::log10(m);
        });
    test_generic_operation_3d(
        "sin", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::sin(m);
        });
    test_generic_operation_3d(
        "cos", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::cos(m);
        });
    test_generic_operation_3d(
        "tan", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::tan(m);
        });
    test_generic_operation_3d(
        "arcsin", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::asin(m);
        });
    test_generic_operation_3d(
        "arccos", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::acos(m);
        });
    test_generic_operation_3d(
        "arctan", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::atan(m);
        });
    test_generic_operation_3d(
        "arcsinh", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::asinh(m);
        });
    test_generic_operation_3d_greater1(
        "arccosh", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::acosh(m);
        });
    test_generic_operation_3d(
        "arctanh", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::atanh(m);
        });

    test_generic_operation_3d(
        "invsqrt", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::invsqrt(m);
        });
    test_generic_operation_3d(
        "invcbrt", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::invcbrt(m);
        });
    test_generic_operation_3d(
        "exp10", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::exp10(m);
        });
    test_generic_operation_3d(
        "erf", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::erf(m);
        });
    test_generic_operation_3d(
        "erfc", [](custom_tensor_type m) -> blaze::DynamicTensor<double> {
            return blaze::erfc(m);
        });
}
#endif

int main(int argc, char* argv[])
{
    test_0d_operations();
    test_1d_operations();
    test_2d_operations();

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_3d_operations();
#endif
    return hpx::util::report_errors();
}
