// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #278: Python to PhySL Translation generates empty block #278

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
#include <cstdint>

void test_0d()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile("shape(1)", snippets);
    auto f = code.run();

    auto list = phylanx::execution_tree::extract_list_value(f());
    HPX_TEST_EQ(list.size(), std::int64_t(0));
}

void test_1d()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code =
        phylanx::execution_tree::compile("shape([1, 2, 3, 4], 0)", snippets);
    auto f = code.run();

    auto list = phylanx::execution_tree::extract_list_value(f());
    HPX_TEST_EQ(list.size(), std::int64_t(1));
    HPX_TEST_EQ(*list.begin(), phylanx::ir::node_data<std::int64_t>(4));
}

void test_1d_axis()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code =
        phylanx::execution_tree::compile("shape([1, 2, 3, 4], 0)", snippets);
    auto f = code.run();

    auto result = f();
    HPX_TEST_EQ(phylanx::execution_tree::extract_integer_value(result),
                phylanx::ir::node_data<std::int64_t>((4)));
}

void test_2d()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(
        "shape([[1, 2, 3, 4], [1, 2, 3, 4]])", snippets);
    auto f = code.run();

    auto list = phylanx::execution_tree::extract_list_value(f());
    HPX_TEST_EQ(list.size(), std::int64_t(2));
    auto it = list.begin();
    HPX_TEST_EQ(*it++, phylanx::ir::node_data<std::int64_t>(2));
    HPX_TEST_EQ(*it++, phylanx::ir::node_data<std::int64_t>(4));
}

void test_2d_x()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(
        "shape([[1, 2, 3, 4], [1, 2, 3, 4]], 0)", snippets);
    auto f = code.run();

    auto result = f();
    HPX_TEST_EQ(phylanx::execution_tree::extract_integer_value(result),
                phylanx::ir::node_data<std::int64_t>((2)));
}

void test_2d_y()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(
        "shape([[1, 2, 3, 4], [1, 2, 3, 4]], 1)", snippets);
    auto f = code.run();

    auto result = f();
    HPX_TEST_EQ(phylanx::execution_tree::extract_integer_value(result),
                phylanx::ir::node_data<std::int64_t>((4)));
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
