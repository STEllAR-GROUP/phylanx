// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #278: Python to PhySL Translation generates empty block #278

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <cstdint>

void test_0d()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile("shape(1)", snippets);

    auto list = phylanx::execution_tree::extract_list_value(f());
    HPX_TEST_EQ(list.size(), std::int64_t(0));
}

void test_1d()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(
        "shape([1, 2, 3, 4], 0)", snippets);

    auto list = phylanx::execution_tree::extract_list_value(f());
    HPX_TEST_EQ(list.size(), std::int64_t(1));
    HPX_TEST_EQ(*list.begin(), std::size_t(4));
}

void test_1d_axis()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(
        "shape([1, 2, 3, 4], 0)", snippets);

    auto result = f();
    HPX_TEST_EQ(phylanx::execution_tree::extract_integer_value(result),
        std::int64_t(4));
}

void test_2d()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(
        "shape([[1, 2, 3, 4], [1, 2, 3, 4]])", snippets);

    auto list = phylanx::execution_tree::extract_list_value(f());
    HPX_TEST_EQ(list.size(), std::int64_t(2));
    auto it = list.begin();
    HPX_TEST_EQ(*it++, std::size_t(2));
    HPX_TEST_EQ(*it++, std::size_t(4));
}

void test_2d_x()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(
        "shape([[1, 2, 3, 4], [1, 2, 3, 4]], 0)", snippets);

    auto result = f();
    HPX_TEST_EQ(phylanx::execution_tree::extract_integer_value(result),
        std::int64_t(2));
}

void test_2d_y()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(
        "shape([[1, 2, 3, 4], [1, 2, 3, 4]], 1)", snippets);

    auto result = f();
    HPX_TEST_EQ(phylanx::execution_tree::extract_integer_value(result),
        std::int64_t(4));
}

int main(int argc, char* argv[])
{
    test_0d();

    test_1d();
    test_1d_axis();

    test_2d();
    test_2d_x();
    test_2d_y();

    return hpx::util::report_errors();
}
