// Copyright (c) 2018 Tianyi Zhang
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
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive generic =
        phylanx::execution_tree::primitives::create_generic_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
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
            std::vector<phylanx::execution_tree::primitive_argument_type>{
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
            std::vector<phylanx::execution_tree::primitive_argument_type>{
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
    test_generic_operation_0d("sin", std::sin);
    test_generic_operation_0d("cos", std::cos);
    test_generic_operation_0d("tan", std::tan);
    test_generic_operation_0d("sinh", std::sinh);
    test_generic_operation_0d("cosh", std::cosh);
    test_generic_operation_0d("tanh", std::tanh);

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

    test_generic_operation_0d("floor", std::floor);
    test_generic_operation_0d("ceil", std::ceil);
    test_generic_operation_0d("trunc", std::trunc);

    test_generic_operation_1d("floor",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::floor(m); });
    test_generic_operation_1d("ceil",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::ceil(m); });
    test_generic_operation_1d("trunc",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::trunc(m); });

    test_generic_operation_2d("floor",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::floor(m); });
    test_generic_operation_2d("ceil",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::ceil(m); });
    test_generic_operation_2d("trunc",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::trunc(m); });

    test_generic_operation_0d("exp", std::exp);
    test_generic_operation_0d("exp2", std::exp2);
    test_generic_operation_0d("log", std::log);
    test_generic_operation_0d("log10", std::log10);
    test_generic_operation_0d("log2", std::log2);

    test_generic_operation_1d("exp",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::exp(m); });
    test_generic_operation_1d("exp2",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::exp2(m); });
    test_generic_operation_1d("log",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::log(m); });
    test_generic_operation_1d("log10",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::log10(m); });
    test_generic_operation_1d("log2",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::log2(m); });

    test_generic_operation_2d("exp",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::exp(m); });
    test_generic_operation_2d("exp2",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::exp2(m); });
    test_generic_operation_2d("log",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::log(m); });
    test_generic_operation_2d("log10",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::log10(m); });
    test_generic_operation_2d("log2",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::log2(m); });

    test_generic_operation_0d("sqrt", std::sqrt);
    test_generic_operation_0d("cbrt", std::cbrt);

    test_generic_operation_1d("sqrt",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::sqrt(m); });
    test_generic_operation_1d("cbrt",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::cbrt(m); });

    test_generic_operation_2d("sqrt",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::sqrt(m); });
    test_generic_operation_2d("cbrt",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::cbrt(m); });

    return hpx::util::report_errors();
}
