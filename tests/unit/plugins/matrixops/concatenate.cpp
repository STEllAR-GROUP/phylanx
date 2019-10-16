// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
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

void test_concatenate_PhySL_2d_axis0()
{
    std::string const code = R"(block(
        define(a, [[1.0, 2.0, 3.0],[6.0, 1.0, 9.0]]),
        define(b, [[0.0, 4.0, 2.0]]),
        concatenate(list(a,b), 0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
            {1.0, 2.0, 3.0}, {6.0, 1.0, 9.0}, {0.0, 4.0, 2.0}}));
}

void test_concatenate_PhySL_2d_axis1()
{
    std::string const code = R"(block(
        define(a, [[1.0, 2.0, 3.0],[6.0, 1.0, 9.0]]),
        define(b, [[0.0, 4.0], [7.0, 5.0]]),
        concatenate(list(a,b), 1))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
            {1.0, 2.0, 3.0, 0.0, 4.0}, {6.0, 1.0, 9.0, 7.0, 5.0}}));
}

void test_concatenate_PhySL_2d_nil()
{
    std::string const code = R"(block(
        define(a, [[1.0, 2.0, 3.0],[6.0, 1.0, 9.0]]),
        define(b, [[0.0, 4.0], [7.0, 5.0]]),
        concatenate(list(a,b), nil))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{
            1.0, 2.0, 3.0, 6.0, 1.0, 9.0, 0.0, 4.0, 7.0, 5.0}));
}

void test_concatenate_PhySL_3d_axis0()
{
    std::string const code = R"(block(
        define(a, [[[1.0, 2.0, 3.0], [6.0, 1.0, 9.0]]]),
        define(b, [[[0.0, 4.0, 4.0], [7.0, 5.0, 1.0]]]),
        concatenate(list(a,b), 0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicTensor<double>{{{{1.0, 2.0, 3.0}, {6.0, 1.0, 9.0}},
                {{0.0, 4.0, 4.0}, {7.0, 5.0, 1.0}}}}));
}

void test_concatenate_PhySL_3d_axis1()
{
    std::string const code = R"(block(
        define(a, [[[1.0, 2.0, 3.0], [6.0, 1.0, 9.0]]]),
        define(b, [[[0.0, 4.0, 4.0], [7.0, 5.0, 1.0]]]),
        concatenate(list(a,b), 1))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicTensor<double>{{{{1.0, 2.0, 3.0}, {6.0, 1.0, 9.0},
                {0.0, 4.0, 4.0}, {7.0, 5.0, 1.0}}}}));
}

void test_concatenate_PhySL_3d_axis2()
{
    std::string const code = R"(block(
        define(a, [[[1.0, 2.0, 3.0], [6.0, 1.0, 9.0]]]),
        define(b, [[[0.0, 4.0, 4.0], [7.0, 5.0, 1.0]]]),
        concatenate(list(a,b), 2))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicTensor<double>{{{{1.0, 2.0, 3.0, 0.0, 4.0, 4.0},
                {6.0, 1.0, 9.0, 7.0, 5.0, 1.0}}}}));
}

void test_concatenate_PhySL_3d_nil()
{
    std::string const code = R"(block(
        define(a, [[[1.0, 2.0, 3.0], [6.0, 1.0, 9.0]]]),
        define(b, [[[0.0, 4.0, 4.0], [7.0, 5.0, 1.0]]]),
        concatenate(list(a,b), nil))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{
            1.0, 2.0, 3.0, 6.0, 1.0, 9.0, 0.0, 4.0, 4.0, 7.0, 5.0, 1.0}));
}

void test_concatenate_1d()
{
    blaze::DynamicVector<double> v1{-1.0, 0.0, 1.0};
    blaze::DynamicVector<double> v2{2.0, 3.0};
    blaze::DynamicVector<double> expected{-1.0, 0.0, 1.0, 2.0, 3.0};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_concatenate(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    phylanx::execution_tree::primitive_arguments_type{
                        phylanx::ir::node_data<double>(v1),
                        phylanx::ir::node_data<double>(v2)}}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_concatenate_2d_axis0()
{
    blaze::DynamicMatrix<double> v1{{-1.0, 0.0, 1.0}, {2.0, 3.0, 8.0}};
    blaze::DynamicMatrix<double> v2{{2.0, 4.0, 5.0}};
    blaze::DynamicMatrix<double> expected{
        {-1.0, 0.0, 1.0}, {2.0, 3.0, 8.0}, {2.0, 4.0, 5.0}};

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_concatenate(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    phylanx::execution_tree::primitive_arguments_type{
                        phylanx::ir::node_data<double>(v1),
                        phylanx::ir::node_data<double>(v2)}}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_concatenate_2d_axis1()
{
    blaze::DynamicMatrix<double> v1{{-1.0, 0.0, 1.0}, {2.0, 3.0, 8.0}};
    blaze::DynamicMatrix<double> v2{{2.0, 9.0}, {3.0, 1.0}};
    blaze::DynamicMatrix<double> expected{
        {-1.0, 0.0, 1.0, 2.0, 9.0}, {2.0, 3.0, 8.0, 3.0, 1.0}};

    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::primitives::create_concatenate(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    phylanx::execution_tree::primitive_arguments_type{
                        phylanx::ir::node_data<double>(v1),
                        phylanx::ir::node_data<double>(v2)}},
                phylanx::execution_tree::primitive_argument_type(
                    phylanx::ir::node_data<std::int64_t>(1))});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f = p.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(expected),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_concatenate_1d();
    test_concatenate_2d_axis0();
    test_concatenate_2d_axis1();
    test_concatenate_PhySL_2d_axis0();
    test_concatenate_PhySL_2d_axis1();
    test_concatenate_PhySL_2d_nil();
    test_concatenate_PhySL_3d_axis0();
    test_concatenate_PhySL_3d_axis1();
    test_concatenate_PhySL_3d_axis2();
    test_concatenate_PhySL_3d_nil();

    return hpx::util::report_errors();
}
