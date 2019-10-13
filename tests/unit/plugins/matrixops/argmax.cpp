// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
void test_argmax_0d()
{
    int s1 = 10;

    std::int64_t expected = 0ul;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(s1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmax(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value_strict(f.get()));
}

///////////////////////////////////////////////////////////////////////////////
void test_argmax_1d()
{
    blaze::DynamicVector<double> v1{1.0, 2.0, 3.0, 1.0};

    std::int64_t expected = 2;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmax(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value_strict(f.get()));
}

///////////////////////////////////////////////////////////////////////////////
void test_argmax_2d_flat()
{
    blaze::DynamicMatrix<double> m1{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};

    std::int64_t expected = 5;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmax(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value_strict(f.get()));
}

void test_argmax_2d_0_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicMatrix<double> m1{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicVector<std::int64_t>{1, 1, 1});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>());

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmax(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}

void test_argmax_2d_1_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicMatrix<double> m1{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicVector<std::int64_t>{2, 2});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmax(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}

///////////////////////////////////////////////////////////////////////////////
void test_argmax_3d_flat()
{
    blaze::DynamicTensor<double> t1{
        {{1.0, 20.0, 3.0}, {40.0, 5.0, 60.0}},
        {{10.0, 2.0, 30.0}, {4.0, 50.0, 6.0}}};

    std::int64_t expected = 5;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmax(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        p.eval();


    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(expected),
        phylanx::execution_tree::extract_integer_value_strict(f.get()));
}

void test_argmax_3d_0_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicTensor<double> t1{
        {{1.0, 20.0, 3.0}, {40.0, 5.0, 60.0}},
        {{10.0, 2.0, 30.0}, {4.0, 50.0, 6.0}}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicMatrix<std::int64_t>{{1, 0, 1}, {0, 1, 0}});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmax(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}

void test_argmax_3d_1_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicTensor<double> t1{
        {{1.0, 20.0, 3.0}, {40.0, 5.0, 60.0}},
        {{10.0, 2.0, 30.0}, {4.0, 50.0, 6.0}}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicMatrix<std::int64_t>{{1, 0, 1}, {0, 1, 0}});

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmax(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}

void test_argmax_3d_2_axis()
{
    using arg_type = phylanx::execution_tree::primitive_argument_type;

    blaze::DynamicTensor<double> t1{
        {{1.0, 20.0, 3.0}, {40.0, 5.0, 60.0}},
        {{10.0, 2.0, 30.0}, {4.0, 50.0, 6.0}}};

    phylanx::ir::node_data<std::int64_t> expected(
        blaze::DynamicMatrix<std::int64_t>({{1, 2}, {2, 1}}));

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(t1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_argmax(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<arg_type> f = p.eval();

    auto actual =
        phylanx::execution_tree::extract_integer_value_strict(f.get());

    HPX_TEST_EQ(expected, actual);
}

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

void test_operation(std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_argmax_0d();

    test_argmax_1d();

    test_argmax_2d_flat();
    test_argmax_2d_0_axis();
    test_argmax_2d_1_axis();

    test_operation("argmax([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], 0)", "[1, 1, 1]");
    test_operation("argmax([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], 1)", "[2, 2]");

    test_argmax_3d_flat();
    test_argmax_3d_0_axis();
    test_argmax_3d_1_axis();
    test_argmax_3d_2_axis();

    return hpx::util::report_errors();
}
