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

void test_pad_0d()
{
    std::string const code = R"(block(
       define(A,1.0),
       pad(A, 1, "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result, phylanx::ir::node_data<double>{1.0});
}

void test_pad_1d_0()
{
    std::string const code = R"(block(
       define(A,[1.0, 2.0, 3.0, 4.0]),
       pad(A, 1, "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{5.0, 1.0, 2.0, 3.0, 4.0, 5.0}));
}

void test_pad_1d_1()
{
    std::string const code = R"(block(
       define(A,[1.0, 2.0, 3.0, 4.0]),
       pad(A, [1], "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{5.0, 1.0, 2.0, 3.0, 4.0, 5.0}));
}

void test_pad_1d_2()
{
    std::string const code = R"(block(
       define(A,[1.0, 2.0, 3.0, 4.0]),
       pad(A, [1,2], "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{5.0, 1.0, 2.0, 3.0, 4.0, 5.0, 5.0}));
}

void test_pad_1d_list0()
{
    std::string const code = R"(block(
       define(A,[1.0, 2.0, 3.0, 4.0]),
       pad(A, list(1, nil), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{5.0, 1.0, 2.0, 3.0, 4.0, 5.0}));
}

void test_pad_1d_list1()
{
    std::string const code = R"(block(
       define(A,[1.0, 2.0, 3.0, 4.0]),
       pad(A, list(1, 2), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{5.0, 1.0, 2.0, 3.0, 4.0, 5.0, 5.0}));
}

void test_pad_1d_list2()
{
    std::string const code = R"(block(
       define(A,[1.0, 2.0, 3.0, 4.0]),
       pad(A, list(1, 2), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{5.0, 1.0, 2.0, 3.0, 4.0, 5.0, 5.0}));
}

void test_pad_1d_list3()
{
    std::string const code = R"(block(
       define(A,[1.0, 2.0, 3.0, 4.0]),
       pad(A, list(list(1, 2),nil), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{5.0, 1.0, 2.0, 3.0, 4.0, 5.0, 5.0}));
}

void test_pad_1d_list3_0()
{
    std::string const code = R"(block(
       define(A,[1.0, 2.0, 3.0, 4.0]),
       pad(A, list(list(1, 2),nil), "constant"))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 0.0, 0.0}));
}

void test_pad_2d_0()
{
    std::string const code = R"(block(
       define(A,[[1.0, 2.0], [3.0, 4.0]]),
       pad(A, 1, "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
            {5.0, 5.0, 5.0, 5.0}, {5.0, 1.0, 2.0, 5.0}, {5.0, 3.0, 4.0, 5.0},
            {5.0, 5.0, 5.0, 5.0}}));
}

void test_pad_2d_1()
{
    std::string const code = R"(block(
       define(A,[[1.0, 2.0], [3.0, 4.0]]),
       pad(A, [1], "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
            {5.0, 5.0, 5.0, 5.0}, {5.0, 1.0, 2.0, 5.0}, {5.0, 3.0, 4.0, 5.0},
            {5.0, 5.0, 5.0, 5.0}}));
}

void test_pad_2d_2()
{
    std::string const code = R"(block(
       define(A,[[1.0, 2.0], [3.0, 4.0]]),
       pad(A, [[1,2]], "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicMatrix<double>{{5.0, 5.0, 5.0, 5.0, 5.0},
                {5.0, 1.0, 2.0, 5.0, 5.0}, {5.0, 3.0, 4.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}}));
}

void test_pad_2d_2_0()
{
    std::string const code = R"(block(
       define(A,[[1.0, 2.0], [3.0, 4.0]]),
       pad(A, [[1,2]], "constant"))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicMatrix<double>{{0.0, 0.0, 0.0, 0.0, 0.0},
                {0.0, 1.0, 2.0, 0.0, 0.0}, {0.0, 3.0, 4.0, 0.0, 0.0},
                {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}}));
}

void test_pad_2d_list0()
{
    std::string const code = R"(block(
       define(A,[[1.0, 2.0], [3.0, 4.0]]),
       pad(A, list(1), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
            {5.0, 5.0, 5.0, 5.0}, {5.0, 1.0, 2.0, 5.0},
            {
                5.0,
                3.0,
                4.0,
                5.0,
            },
            {5.0, 5.0, 5.0, 5.0}}));
}

void test_pad_2d_list1()
{
    std::string const code = R"(block(
       define(A,[[1.0, 2.0], [3.0, 4.0]]),
       pad(A, list(1,nil), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
            {5.0, 5.0, 5.0, 5.0}, {5.0, 1.0, 2.0, 5.0},
            {
                5.0,
                3.0,
                4.0,
                5.0,
            },
            {5.0, 5.0, 5.0, 5.0}}));
}

void test_pad_2d_list2()
{
    std::string const code = R"(block(
       define(A,[[1.0, 2.0], [3.0, 4.0]]),
       pad(A, list(1,2), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicMatrix<double>{{5.0, 5.0, 5.0, 5.0, 5.0},
                {5.0, 1.0, 2.0, 5.0, 5.0}, {5.0, 3.0, 4.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}}));
}

void test_pad_3d_0()
{
    std::string const code = R"(block(
       define(A,[[[1.0, 2.0]]]),
       pad(A, 1, "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 1.0, 2.0, 5.0}, {5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0}}}));
}

void test_pad_3d_1()
{
    std::string const code = R"(block(
       define(A,[[[1.0, 2.0]]]),
       pad(A, [1,2], "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 1.0, 2.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}}}));
}

void test_pad_3d_2()
{
    std::string const code = R"(block(
       define(A,[[[1.0, 2.0]]]),
       pad(A, [[1,2],[0,1],[1,0]], "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicTensor<double>{{{5.0, 5.0, 5.0}, {5.0, 5.0, 5.0}},
                {{5.0, 1.0, 2.0}, {5.0, 5.0, 5.0}},
                {{5.0, 5.0, 5.0}, {5.0, 5.0, 5.0}},
                {{5.0, 5.0, 5.0}, {5.0, 5.0, 5.0}}}));
}

void test_pad_3d_list0()
{
    std::string const code = R"(block(
       define(A,[[[1.0, 2.0]],[[3.0, 4.0]]]),
       pad(A, list(1), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 1.0, 2.0, 5.0}, {5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 3.0, 4.0, 5.0}, {5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0}}}));
}

void test_pad_3d_list1()
{
    std::string const code = R"(block(
       define(A,[[[1.0, 2.0]],[[3.0, 4.0]]]),
       pad(A, list(1,nil), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 1.0, 2.0, 5.0}, {5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 3.0, 4.0, 5.0}, {5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0}}}));
}

void test_pad_3d_list2()
{
    std::string const code = R"(block(
       define(A,[[[1.0, 2.0]],[[3.0, 4.0]]]),
       pad(A, list(list(1,2), nil), "constant", 5.0))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 1.0, 2.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 3.0, 4.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}},
            {{5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0},
                {5.0, 5.0, 5.0, 5.0, 5.0}, {5.0, 5.0, 5.0, 5.0, 5.0}}}));
}

void test_pad_3d_list2_0()
{
    std::string const code = R"(block(
       define(A,[[[1.0, 2.0]],[[3.0, 4.0]]]),
       pad(A, list(list(1,2), nil), "constant"))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicTensor<double>{
            {{0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0},
                {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
            {{0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 1.0, 2.0, 0.0, 0.0},
                {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
            {{0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 3.0, 4.0, 0.0, 0.0},
                {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
            {{0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0},
                {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}},
            {{0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0},
                {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}}}));
}

int main(int argc, char* argv[])
{
    test_pad_0d();
    test_pad_1d_0();
    test_pad_1d_1();
    test_pad_1d_2();
    test_pad_1d_list0();
    test_pad_1d_list1();
    test_pad_1d_list2();
    test_pad_1d_list3();
    test_pad_1d_list3_0();
    test_pad_2d_0();
    test_pad_2d_1();
    test_pad_2d_2();
    test_pad_2d_0();
    test_pad_2d_list0();
    test_pad_2d_list1();
    test_pad_2d_list2();
    test_pad_3d_0();
    test_pad_3d_1();
    test_pad_3d_2();
    test_pad_3d_list0();
    test_pad_3d_list1();
    test_pad_3d_list2();
    test_pad_3d_list2_0();

    return hpx::util::report_errors();
}
