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

void test_insert_PhySL_0d_0()
{
    std::string const code = R"(block(
        define(a, 7.0),
        define(b, 1),
        define(c, 5.0),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{7.0, 5.0}));
}

void test_insert_PhySL_0d_0_nil()
{
    std::string const code = R"(block(
        define(a, 7.0),
        define(b, [0, 0, 1, 0, 1]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c, nil))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{3.0, 4.0, 6.0, 7.0, 5.0, 2.0}));
}

void test_insert_PhySL_0d_1()
{
    std::string const code = R"(block(
        define(a, 7.0),
        define(b, 1),
        define(c, [5.0, 6.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{7.0, 5.0, 6.0}));
}

void test_insert_PhySL_0d_2()
{
    std::string const code = R"(block(
        define(a, 7.0),
        define(b, [0, 1, 0]),
        define(c, 3.0),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{3.0, 3.0, 7.0, 3.0}));
}

void test_insert_PhySL_0d_3()
{
    std::string const code = R"(block(
        define(a, 7.0),
        define(b, [0, 0, 1, 0, -4]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{7.0, 3.0, 4.0, 6.0, 7.0, 5.0}));
}

void test_insert_PhySL_0d_4()
{
    std::string const code = R"(block(
        define(a, 7.0),
        define(b, [0, -1, 1, 0, -1]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{3.0, 4.0, 6.0, 2.0, 7.0, 5.0}));
}

void test_insert_PhySL_1d_0()
{
    std::string const code = R"(block(
        define(a, [7.0]),
        define(b, 1),
        define(c, [5.0, 6.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{7.0, 5.0, 6.0}));
}

void test_insert_PhySL_1d_1()
{
    std::string const code = R"(block(
        define(a, [7.0, 8.0, 9.0]),
        define(b, [0, 0, 1, 0, 1]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{
            3.0, 4.0, 6.0, 7.0, 5.0, 2.0, 8.0, 9.0}));
}

void test_insert_PhySL_2d_1()
{
    std::string const code = R"(block(
        define(a, [[7.0, 8.0, 9.0]]),
        define(b, [0, 0, 1, 0, 1]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{
            3.0, 4.0, 6.0, 7.0, 5.0, 2.0, 8.0, 9.0}));
}

void test_insert_PhySL_2d_2()
{
    std::string const code = R"(block(
        define(a, [[7.0, 8.0, 9.0]]),
        define(b, [0, 0, -1, 0, -2]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{
            3.0, 4.0, 6.0, 7.0, 2.0, 8.0, 5.0, 9.0}));
}

void test_insert_PhySL_2d_3()
{
    std::string const code = R"(block(
        define(a, [[7.0, 8.0, 1.0], [9.0, 10.0, 7.0]]),
        define(b, [0, 0, -1, 0, -2]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{
            3.0, 4.0, 6.0, 7.0, 8.0, 1.0, 9.0, 2.0, 10.0, 5.0, 7.0}));
}

void test_insert_PhySL_2d_axis0_0()
{
    std::string const code = R"(block(
        define(a, [[7.0, 8.0, 9.0],[10.0, 11.0, 12.0]]),
        define(b, 1),
        define(c, [3.0, 4.0, 5.0]),
        insert(a, b, c, 0))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
            {7.0, 8.0, 9.0}, {3.0, 4.0, 5.0}, {10.0, 11.0, 12.0}}));
}

void test_insert_PhySL_2d_axis0_1()
{
    std::string const code = R"(block(
        define(a, [[7.0, 8.0, 9.0],[10.0, 11.0, 12.0]]),
        define(b, [0, 0, 1, 0, -3]),
        define(c, [3.0, 4.0, 5.0]),
        insert(a, b, c, 0))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
            {7.0, 8.0, 9.0}, {3.0, 4.0, 5.0}, {3.0, 4.0, 5.0}, {3.0, 4.0, 5.0},
            {10.0, 11.0, 12.0}, {3.0, 4.0, 5.0}, {3.0, 4.0, 5.0}}));
}

void test_insert_PhySL_2d_axis1_0()
{
    std::string const code = R"(block(
        define(a, [[7.0, 8.0, 9.0],[10.0, 11.0, 12.0]]),
        define(b, [0, 0, 1, 0, -1]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c, 1))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
            {3.0, 4.0, 6.0, 7.0, 5.0, 8.0, 2.0, 9.0},
            {3.0, 4.0, 6.0, 10.0, 5.0, 11.0, 2.0, 12.0}}));
}

void test_insert_PhySL_2d_axis1_1()
{
    std::string const code = R"(block(
        define(a, [[7.0, 8.0], [9.0, 10.0], [11.0, 12.0]]),
        define(b, [0, 1]),
        define(c, [3.0]),
        insert(a, b, c, 1))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicMatrix<double>{{{3.0, 7.0, 3.0, 8.0},
                {3.0, 9.0, 3.0, 10.0}, {3.0, 11.0, 3.0, 12.0}}}));
}

void test_insert_PhySL_3d_1()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0, 9.0]]]),
        define(b, [0, 0, 1, 0, 1]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{
            3.0, 4.0, 6.0, 7.0, 5.0, 2.0, 8.0, 9.0}));
}

void test_insert_PhySL_3d_2()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0, 9.0]]]),
        define(b, [0, 0, -1, 0, -2]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{
            3.0, 4.0, 6.0, 7.0, 2.0, 8.0, 5.0, 9.0}));
}

void test_insert_PhySL_3d_3()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0, 11.0], [9.0, 10.0, 12.0]]]),
        define(b, [0, 0, -1, 0, 3]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{
            3.0, 4.0, 6.0, 7.0, 8.0, 11.0, 2.0, 9.0, 10., 5.0, 12.0}));
}

void test_insert_PhySL_3d_axis0_0()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0, 11.0], [9.0, 10.0, 12.0]]]),
        define(b, 1),
        define(c, [3.0, 4.0, 5.0]),
        insert(a, b, c, 0))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicTensor<double>{{{7.0, 8.0, 11.0}, {9.0, 10., 12.0}},
                {{3.0, 4.0, 5.0}, {3.0, 4.0, 5.0}}}));
}

void test_insert_PhySL_3d_axis0_1()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0], [9.0, 10.0]], [[11.0, 12.0], [13.0, 14.0]]]),
        define(b, [0, 0, -1]),
        define(c, [3.0, 4.0]),
        insert(a, b, c, 0))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicTensor<double>{{{{3.0, 4.0}, {3.0, 4.0}},
                {{3.0, 4.0}, {3.0, 4.0}}, {{7.0, 8.0}, {9.0, 10.}},
                {{3.0, 4.0}, {3.0, 4.0}}, {{11.0, 12.0}, {13.0, 14.0}}}}));
}

void test_insert_PhySL_3d_axis0_2()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0, 9.0], [10.0, 11.0, 12.0], [17.0, 18.0, 19.0],
              [20.0, 21.0, 22.0]]]),
        define(b, [0, -1, 0, 1]),
        define(c, [3.0, 4.0, 5.0]),
        insert(a, b, c, 0))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));
    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicTensor<double>{{{{3.0, 4.0, 5.0}, {3.0, 4.0, 5.0},
                                              {3.0, 4.0, 5.0}, {3.0, 4.0, 5.0}},
                {{3.0, 4.0, 5.0}, {3.0, 4.0, 5.0}, {3.0, 4.0, 5.0},
                    {3.0, 4.0, 5.0}},
                {{3.0, 4.0, 5.0}, {3.0, 4.0, 5.0}, {3.0, 4.0, 5.0},
                    {3.0, 4.0, 5.0}},
                {{7.0, 8.0, 9.0}, {10., 11.0, 12.0}, {17.0, 18.0, 19.0},
                    {20., 21.0, 22.0}},
                {{3.0, 4.0, 5.0}, {3.0, 4.0, 5.0}, {3.0, 4.0, 5.0},
                    {3.0, 4.0, 5.0}}}}));
}

void test_insert_PhySL_3d_axis1_0()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0, 9.0],[10.0, 11.0, 12.0]]]),
        define(b, [0, 1, 0]),
        define(c, [3.0, 4.0, 5.0]),
        insert(a, b, c, 1))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicTensor<double>{{{{3.0, 4.0, 5.0}, {3.0, 4.0, 5.0},
                {7.0, 8.0, 9.0}, {3.0, 4.0, 5.0}, {10.0, 11.0, 12.0}}}}));
}

void test_insert_PhySL_3d_axis1_1()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0], [9.0, 10.0]], [[11.0, 12.0], [13.0, 14.0]]]),
        define(b, [0, 0]),
        define(c, [3.0, 4.0]),
        insert(a, b, c, 1))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{{3.0, 4.0}, {3.0, 4.0}, {7.0, 8.0}, {9.0, 10.0}},
                {{3.0, 4.0}, {3.0, 4.0}, {11.0, 12.0}, {13.0, 14.0}}}}));
}

void test_insert_PhySL_3d_axis1_2()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0, 9.0], [10.0, 11.0, 12.0]], [[17.0, 18.0, 19.0],
              [20.0, 21.0, 22.0]]]),
        define(b, [0, -2, 0]),
        define(c, [3.0, 4.0, 5.0]),
        insert(a, b, c, 1))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{{3.0, 4.0, 5.0}, {3.0, 4.0, 5.0}, {3.0, 4.0, 5.0},
                 {7.0, 8.0, 9.0}, {10.0, 11.0, 12.0}},
                {{3.0, 4.0, 5.0}, {3.0, 4.0, 5.0}, {3.0, 4.0, 5.0},
                    {17.0, 18.0, 19.0}, {20.0, 21.0, 22.0}}}}));
}

void test_insert_PhySL_3d_axis2_0()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0, 9.0], [10.0, 11.0, 12.0]], [[17.0, 18.0, 19.0],
              [20.0, 21.0, 22.0]]]),
        define(b, [0, 1]),
        define(c, [3.0]),
        insert(a, b, c, 2))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));
    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{{3.0, 7.0, 3.0, 8.0, 9.0}, {3.0, 10.0, 3.0, 11.0, 12.0}},
                {{3.0, 17.0, 3.0, 18.0, 19.0},
                    {3.0, 20.0, 3.0, 21.0, 22.0}}}}));
}

void test_insert_PhySL_3d_axis2_1()
{
    std::string const code = R"(block(
        define(a, [[[7.0, 8.0, 9.0], [10.0, 11.0, 12.0], [17.0, 18.0, 19.0],
              [20.0, 21.0, 22.0]]]),
        define(b, [0, 0, -1, 0, 3]),
        define(c, [3.0, 4.0, 5.0, 6.0, 2.0]),
        insert(a, b, c, 2))
    )";

    auto result = phylanx::execution_tree::extract_numeric_value_strict(
        compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{{3.0, 4.0, 6.0, 7.0, 8.0, 5.0, 9.0, 2.0},
                {3.0, 4.0, 6.0, 10.0, 11.0, 5.0, 12.0, 2.0},
                {3.0, 4.0, 6.0, 17.0, 18.0, 5.0, 19.0, 2.0},
                {3.0, 4.0, 6.0, 20.0, 21.0, 5.0, 22.0, 2.0}}}}));
}

int main(int argc, char* argv[])
{
    test_insert_PhySL_0d_0();
    test_insert_PhySL_0d_0_nil();
    test_insert_PhySL_0d_1();
    test_insert_PhySL_0d_2();
    test_insert_PhySL_0d_3();
    test_insert_PhySL_0d_4();
    test_insert_PhySL_1d_0();
    test_insert_PhySL_1d_1();
    test_insert_PhySL_2d_1();
    test_insert_PhySL_2d_2();
    test_insert_PhySL_2d_3();
    test_insert_PhySL_2d_axis0_0();
    test_insert_PhySL_2d_axis0_1();
    test_insert_PhySL_2d_axis1_0();
    test_insert_PhySL_2d_axis1_1();
    test_insert_PhySL_3d_1();
    test_insert_PhySL_3d_2();
    test_insert_PhySL_3d_3();
    test_insert_PhySL_3d_axis0_0();
    test_insert_PhySL_3d_axis0_1();
    test_insert_PhySL_3d_axis1_0();
    test_insert_PhySL_3d_axis1_1();
    test_insert_PhySL_3d_axis1_2();
    test_insert_PhySL_3d_axis2_0();
    test_insert_PhySL_3d_axis2_1();

    return hpx::util::report_errors();
}
