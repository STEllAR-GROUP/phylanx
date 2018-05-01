// Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// #358: Slice should support nil as an argument

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& code)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    return phylanx::execution_tree::compile(code, snippets, env);
}

void test_slice_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile(code)(), compile(expected_str)());
}

///////////////////////////////////////////////////////////////////////////////
void test_nil_arguments()
{
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], nil)",
        "[1, 2, 3, 4, 5, 6, 7, 8]");

    test_slice_operation(
        "slice([[1, 2, 3, 4], [5, 6, 7, 8]], nil)",
        "[[1, 2, 3, 4], [5, 6, 7, 8]]");
    test_slice_operation(
        "slice([[1, 2, 3, 4], [5, 6, 7, 8]], nil, nil)",
        "[[1, 2, 3, 4], [5, 6, 7, 8]]");

    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), nil)",
        "make_list(1, 2, 3, 4, 5, 6, 7, 8)");
}

void test_empty_list_arguments()
{
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list())",
        "[1, 2, 3, 4, 5, 6, 7, 8]");

    test_slice_operation(
        "slice([[1, 2, 3, 4], [5, 6, 7, 8]], make_list())",
        "[[1, 2, 3, 4], [5, 6, 7, 8]]");
    test_slice_operation(
        "slice([[1, 2, 3, 4], [5, 6, 7, 8]], make_list(), make_list())",
        "[[1, 2, 3, 4], [5, 6, 7, 8]]");

    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list())",
        "make_list(1, 2, 3, 4, 5, 6, 7, 8)");
}

void test_nil_indices_vector()
{
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(1))",
        "[2, 3, 4, 5, 6, 7, 8]");

    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(nil, nil))",
        "[1, 2, 3, 4, 5, 6, 7, 8]");
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(1, nil))",
        "[2, 3, 4, 5, 6, 7, 8]");
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(nil, 4))",
        "[1, 2, 3, 4]");

    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(nil, nil, nil))",
        "[1, 2, 3, 4, 5, 6, 7, 8]");
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(1, nil, nil))",
        "[2, 3, 4, 5, 6, 7, 8]");
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(1, nil, 2))",
        "[2, 4, 6, 8]");
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(1, 4, nil))",
        "[2, 3, 4]");
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(nil, 4, nil))",
        "[1, 2, 3, 4]");
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(nil, nil, 2))",
        "[1, 3, 5, 7]");
    test_slice_operation(
        "slice([1, 2, 3, 4, 5, 6, 7, 8], make_list(nil, 4, 2))",
        "[1, 3]");
}

void test_nil_indices_matrix()
{
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(nil))",
        "[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(nil), make_list(nil))",
        "[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");

    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(nil, nil))",
        "[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(nil, nil), make_list(nil, nil))",
        "[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");

    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(1, nil))",
        "[[4, 5, 6], [7, 8, 9]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(1, nil), make_list(1, nil))",
        "[[5, 6], [8, 9]]");

    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(nil, 1))",
        "[[1, 2, 3]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(nil, 1), make_list(nil, 1))",
        "[[1]]");

    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(nil, nil, nil))",
        "[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(nil, nil, nil), make_list(nil, nil, nil))",
        "[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(1, nil, nil))",
        "[[4, 5, 6], [7, 8, 9]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(1, nil, nil), make_list(1, nil, nil))",
        "[[5, 6], [8, 9]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(1, nil, 2))",
        "[[4, 5, 6]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(1, nil, 2), make_list(1, nil, 2))",
        "[[5]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(0, 2, nil))",
        "[[1, 2, 3], [4, 5, 6]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(0, 2, nil), make_list(0, 2, nil))",
        "[[1, 2], [4, 5]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(nil, 2, nil))",
        "[[1, 2, 3], [4, 5, 6]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(nil, 2, nil), make_list(nil, 2, nil))",
        "[[1, 2], [4, 5]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(nil, nil, 2))",
        "[[1, 2, 3], [7, 8, 9]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(nil, nil, 2), make_list(nil, nil, 2))",
        "[[1, 3], [7, 9]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], make_list(nil, 2, 2))",
        "[[1, 2, 3]]");
    test_slice_operation(
        "slice([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "
            "make_list(nil, 2, 2), make_list(nil, 2, 2))",
        "[[1]]");
}

void test_nil_indices_list()
{
    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(1))",
        "make_list(2, 3, 4, 5, 6, 7, 8)");

    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(nil, nil))",
        "make_list(1, 2, 3, 4, 5, 6, 7, 8)");
    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(1, nil))",
        "make_list(2, 3, 4, 5, 6, 7, 8)");
    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(nil, 4))",
        "make_list(1, 2, 3, 4)");

    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(nil, nil, nil))",
        "make_list(1, 2, 3, 4, 5, 6, 7, 8)");
    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(1, nil, nil))",
        "make_list(2, 3, 4, 5, 6, 7, 8)");
    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(1, nil, 2))",
        "make_list(2, 4, 6, 8)");
    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(1, 4, nil))",
        "make_list(2, 3, 4)");
    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(nil, 4, nil))",
        "make_list(1, 2, 3, 4)");
    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(nil, nil, 2))",
        "make_list(1, 3, 5, 7)");
    test_slice_operation(
        "slice(make_list(1, 2, 3, 4, 5, 6, 7, 8), make_list(nil, 4, 2))",
        "make_list(1, 3)");
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_nil_arguments();
    test_empty_list_arguments();

    test_nil_indices_vector();
    test_nil_indices_matrix();
    test_nil_indices_list();

    return hpx::util::report_errors();
}
