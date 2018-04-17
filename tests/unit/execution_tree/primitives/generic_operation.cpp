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
    blaze::DynamicVector<double> n = gen.generate(42UL);
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
    blaze::DynamicMatrix<double> n = gen.generate(42UL, 42UL);
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
    test_generic_operation_0d("log", std::log);
    test_generic_operation_1d("log",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::log(m); });
    test_generic_operation_2d("log",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::log(m); });
    test_generic_operation_0d("exp", std::exp);
    test_generic_operation_1d("exp",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::exp(m); });
    test_generic_operation_2d("exp",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::exp(m); });
    test_generic_operation_0d("sin", std::sin);
    test_generic_operation_1d("sin",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::sin(m); });
    test_generic_operation_2d("sin",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::sin(m); });
    test_generic_operation_0d("sinh", std::sinh);
    test_generic_operation_1d("sinh",
        [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicVector<double> { return blaze::sinh(m); });
    test_generic_operation_2d("sinh",
        [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
            -> blaze::DynamicMatrix<double> { return blaze::sinh(m); });
    test_generic_operation_0d("cos", std::cos);
    test_generic_operation_1d("cos",
                              [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
                                      -> blaze::DynamicVector<double> { return blaze::cos(m); });
    test_generic_operation_2d("cos",
                              [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
                                      -> blaze::DynamicMatrix<double> { return blaze::cos(m); });
    test_generic_operation_0d("tan", blaze::tan);
    test_generic_operation_1d("tan",
                              [](blaze::CustomVector<double, blaze::aligned, blaze::padded> m)
                                      -> blaze::DynamicVector<double> { return blaze::tan(m); });
    test_generic_operation_2d("tan",
                              [](blaze::CustomMatrix<double, blaze::aligned, blaze::padded> m)
                                      -> blaze::DynamicMatrix<double> { return blaze::tan(m); });

    return hpx::util::report_errors();
}
