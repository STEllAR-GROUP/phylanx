// Copyright (c) 2018 Tianyi Zhang
// Copyright (c) 2018 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <blaze/Blaze.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

void test_generic_operation_0d(std::string const& func_name,
    double func(double))
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.5));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)},
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    HPX_TEST_EQ(
        func(5.0), phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_generic_operation_1d(std::string const& func_name,
    blaze::DynamicVector<double>
        func(blaze::CustomVector<double, blaze::aligned, blaze::padded>))
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> n = gen.generate(22UL);
    blaze::CustomVector<double, true, true> m(n.data(), n.size(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicVector<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_generic_operation_1d_greater1(std::string const& func_name,
    blaze::DynamicVector<double>
        func(blaze::CustomVector<double, blaze::aligned, blaze::padded>))
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> n = gen.generate(22UL, 1, 5);
    blaze::CustomVector<double, true, true> m(n.data(), n.size(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicVector<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_generic_operation_2d(std::string const& func_name,
    blaze::DynamicMatrix<double>
        func(blaze::CustomMatrix<double, blaze::aligned, blaze::padded>))
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> n = gen.generate(22UL, 22UL);
    blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m(
        n.data(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicMatrix<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_generic_operation_2d_greater1(std::string const& func_name,
    blaze::DynamicMatrix<double>
        func(blaze::CustomMatrix<double, blaze::aligned, blaze::padded>))
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> n = gen.generate(22UL, 22UL, 1, 5);
    blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m(
        n.data(), n.rows(), n.columns(), n.spacing());

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        generic.eval();
    blaze::DynamicMatrix<double> expected = func(m);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
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
    test_generic_operation_0d("sinh", std::sinh);
    test_generic_operation_0d("cosh", std::cosh);
    test_generic_operation_0d("tanh", std::tanh);
    test_generic_operation_0d("arcsin", std::asin);
    test_generic_operation_0d("arccos", std::acos);
    test_generic_operation_0d("arctan", std::atan);
    test_generic_operation_0d("arcsinh", std::asinh);
    test_generic_operation_0d_greater1("arccosh", std::acosh);
    test_generic_operation_0d("arctanh", std::atanh);

    test_generic_operation_1d("amin",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> {
            return blaze::DynamicVector<double>(1, (blaze::min)(m));
        });
    test_generic_operation_1d("amax",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> {
            return blaze::DynamicVector<double>(1, (blaze::max)(m));
        });
    test_generic_operation_1d("absolute",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::abs(m); });
    test_generic_operation_1d("floor",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::floor(m); });
    test_generic_operation_1d("ceil",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::ceil(m); });
    test_generic_operation_1d("trunc",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::trunc(m); });
    test_generic_operation_1d("rint",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::round(m); });
    test_generic_operation_1d("conj",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::conj(m); });
    test_generic_operation_1d("real",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::real(m); });
    test_generic_operation_1d("imag",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::imag(m); });
    test_generic_operation_1d("sqrt",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::sqrt(m); });
    test_generic_operation_1d("cbrt",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::cbrt(m); });
    test_generic_operation_1d("exp",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::exp(m); });
    test_generic_operation_1d("exp2",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::exp2(m); });
    test_generic_operation_1d("log",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::log(m); });
    test_generic_operation_1d("log2",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::log2(m); });
    test_generic_operation_1d("log10",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::log10(m); });
    test_generic_operation_1d("sin",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::sin(m); });
    test_generic_operation_1d("cos",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::cos(m); });
    test_generic_operation_1d("tan",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::tan(m); });
    test_generic_operation_1d("sinh",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
        -> blaze::DynamicVector<double> { return blaze::sinh(m); });
    test_generic_operation_1d("cosh",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
        -> blaze::DynamicVector<double> { return blaze::cosh(m); });
    test_generic_operation_1d("tanh",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
        -> blaze::DynamicVector<double> { return blaze::tanh(m); });
    test_generic_operation_1d("arcsin",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::asin(m); });
    test_generic_operation_1d("arccos",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::acos(m); });
    test_generic_operation_1d("arctan",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::atan(m); });
    test_generic_operation_1d("arcsinh",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::asinh(m); });
    test_generic_operation_1d_greater1("arccosh",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::acosh(m); });
    test_generic_operation_1d("arctanh",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::atanh(m); });

    test_generic_operation_2d("amin",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> {
            return blaze::DynamicMatrix<double>(1, 1, (blaze::min)(m));
        });
    test_generic_operation_2d("amax",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> {
            return blaze::DynamicMatrix<double>(1, 1, (blaze::max)(m));
        });
    test_generic_operation_2d("absolute",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::abs(m); });
    test_generic_operation_2d("floor",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::floor(m); });
    test_generic_operation_2d("ceil",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::ceil(m); });
    test_generic_operation_2d("trunc",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::trunc(m); });
    test_generic_operation_2d("rint",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::round(m); });
    test_generic_operation_2d("conj",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::conj(m); });
    test_generic_operation_2d("real",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::real(m); });
    test_generic_operation_2d("imag",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::imag(m); });
    test_generic_operation_2d("sqrt",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::sqrt(m); });
    test_generic_operation_2d("cbrt",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::cbrt(m); });
    test_generic_operation_2d("exp",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::exp(m); });
    test_generic_operation_2d("exp2",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::exp2(m); });
    test_generic_operation_2d("log",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::log(m); });
    test_generic_operation_2d("log2",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::log2(m); });
    test_generic_operation_2d("log10",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::log10(m); });
    test_generic_operation_2d("sin",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::sin(m); });
    test_generic_operation_2d("cos",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::cos(m); });
    test_generic_operation_2d("tan",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::tan(m); });
    test_generic_operation_2d("sinh",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
        -> blaze::DynamicMatrix<double> { return blaze::sinh(m); });
    test_generic_operation_2d("cosh",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
        -> blaze::DynamicMatrix<double> { return blaze::cosh(m); });
    test_generic_operation_2d("tanh",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
        -> blaze::DynamicMatrix<double> { return blaze::tanh(m); });
    test_generic_operation_2d("arcsin",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::asin(m); });
    test_generic_operation_2d("arccos",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::acos(m); });
    test_generic_operation_2d("arctan",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::atan(m); });
    test_generic_operation_2d("arcsinh",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::asinh(m); });
    test_generic_operation_2d_greater1("arccosh",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::acosh(m); });
    test_generic_operation_2d("arctanh",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::atanh(m); });

    test_generic_operation_0d("invsqrt", blaze::invsqrt);
    test_generic_operation_0d("invcbrt", blaze::invcbrt);
    test_generic_operation_0d(
        "exp10", [](double m) -> double { return std::pow(10, m); });
    test_generic_operation_0d("erf", blaze::erf);
    test_generic_operation_0d("erfc", blaze::erfc);

    test_generic_operation_1d("invsqrt",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::invsqrt(m); });
    test_generic_operation_1d("invcbrt",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::invcbrt(m); });
    test_generic_operation_1d("exp10",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::exp10(m); });
    test_generic_operation_1d("erf",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::erf(m); });
    test_generic_operation_1d("erfc",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::erfc(m); });

    test_generic_operation_2d("invsqrt",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::invsqrt(m); });
    test_generic_operation_2d("invcbrt",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::invcbrt(m); });
    test_generic_operation_2d("exp10",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::exp10(m); });
    test_generic_operation_2d("erf",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::erf(m); });
    test_generic_operation_2d("erfc",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::erfc(m); });

    test_generic_operation_1d("normalize",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::normalize(m); });

    test_generic_operation_0d("trace", std::abs);
    test_generic_operation_2d("trace",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> {
            return blaze::DynamicMatrix<double>(1, 1, blaze::trace(m));
        });

    return hpx::util::report_errors();
}
