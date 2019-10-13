// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
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

phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

void test_clip_PhySL_0()
{
    std::string const code = R"(block(
        define(a, 3),
        define(b, [0, 4, -1]),
        define(c, [[5, 0, 2], [0, 0, 1]]),
        clip(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_integer_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<std::int64_t>(
            blaze::DynamicMatrix<std::int64_t>{{3, 4, 2}, {0, 4, 1}}));
}

void test_clip_PhySL_1()
{
    std::string const code = R"(block(
        define(a, [6.0, 1.0, 9.0]),
        define(b, [0.0, 4.0, 2.0]),
        clip(a, b, nil))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{6.0, 4.0, 9.0}));
}

void test_clip_PhySL_2()
{
    std::string const code = R"(block(
        define(a, [[1.0, 2.0, 3.0],[6.0, 1.0, 9.0]]),
        define(b, [0.0, 4.0, 2.0]),
        clip(a, nil, b))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicMatrix<double>{{0.0, 2.0, 2.0}, {0.0, 1.0, 2.0}}));
}

void test_clip_0d()
{
    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(10.0));

    phylanx::execution_tree::primitive min =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(12.0));

    phylanx::execution_tree::primitive max =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(14.0));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_clip(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg), std::move(min), std::move(max)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(12.0),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_clip_1d()
{
    blaze::DynamicVector<double> v1{-1.0, 0.0, 1.0, 2.0, 4.0, 5.0};
    blaze::DynamicVector<double> v2{3.0, 1.0, -1.0, 2.0, 3.0, 4.0};
    blaze::DynamicVector<double> expected{3.0, 1.0, 1.0, 2.0, 4.0, 5.0};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive min =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive max =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(9.0));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_clip(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg), std::move(min), std::move(max)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_clip_2d()
{
    blaze::DynamicMatrix<double> v1{
        {-1.0, 0.0, 1.0, 2.0, 4.0, 5.0}, {3.0, 1.0, -1.0, 2.0, 3.0, 8.0}};
    blaze::DynamicVector<double> v2{2.0, -1.0, 3.0, 2.0, 5.0, 8.0};
    blaze::DynamicMatrix<double> expected{
        {1.5, 1.5, 1.5, 2.0, 4.0, 5.0}, {2.0, 1.5, 1.5, 2.0, 3.0, 8.0}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive min =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.5));

    phylanx::execution_tree::primitive max =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_clip(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg), std::move(min), std::move(max)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_clip_3d()
{
    blaze::DynamicTensor<double> v1{{{-1.0, 0.0, 1.0}, {2.0, 4.0, 5.0}},
        {{3.0, 1.0, -1.0}, {2.0, 3.0, 8.0}},
        {{1.0, 5.0, 0.0}, {9.0, 3.0, 1.0}}};
    blaze::DynamicTensor<double> expected{{{1.5, 1.5, 1.5}, {2.0, 4.0, 4.0}},
        {{3.0, 1.5, 1.5}, {2.0, 3.0, 4.0}}, {{1.5, 4.0, 1.5}, {4.0, 3.0, 1.5}}};

    phylanx::execution_tree::primitive arg =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive min =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(1.5));

    phylanx::execution_tree::primitive max =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(4.0));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_clip(hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg), std::move(min), std::move(max)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_clip_0d();
    test_clip_1d();
    test_clip_2d();
    test_clip_3d();

    test_clip_PhySL_0();
    test_clip_PhySL_1();
    test_clip_PhySL_2();

    return hpx::util::report_errors();
}
