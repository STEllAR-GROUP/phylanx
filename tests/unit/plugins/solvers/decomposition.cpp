// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2017-2018 Shahrzad Shirzad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
void test_decomposition_lu_PhySL()
{
    std::string const lu_code = R"(block(
        define(A, [[10, -10, 0], [-3, 15, 6], [5, 7, 5]]),
        define(b, lu((A))),
        define(L,slice(b,0)),
        define(U,slice(b,1)),
        define(P,slice(b,2)),
        define(result, false),
        if (all(A == dot(dot(L, U), P)), store(result, true), store(result, false)),
            if ((all(L == [[10, 0, 0], [-3, 12, 0], [5, 12, -1]]) &&
            all(U == [[1, -1, 0], [0, 1, 0.5], [0, 0, 1]]))
            && (all(P == [[1, 0, 0], [0, 1, 0], [0, 0, 1]])),
            store(result, true), store(result, false)),
            result)
    )";

    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(lu_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value_scalar(f()), 1);
}

void test_decomposition(std::string const& func_name)
{
    phylanx::execution_tree::primitive m =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{blaze::DynamicMatrix<double>{
                {10, -10, 0}, {-3, 15, 6}, {5, 7, 5}}});
    phylanx::execution_tree::primitive decomposition =
        phylanx::execution_tree::primitives::create_decomposition(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(m)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        decomposition.eval();
    auto result = phylanx::execution_tree::extract_list_value(f.get());

    auto it = result.begin();
    HPX_TEST_EQ(phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
                    {10, 0, 0}, {-3, 12, 0}, {5, 12, -1}}),
        *it++);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
                    {1, -1, 0}, {0, 1, 0.5}, {0, 0, 1}}),
        *it++);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
                    {1, 0, 0}, {0, 1, 0}, {0, 0, 1}}),
        *it);
}

int main()
{
    test_decomposition_lu_PhySL();
    test_decomposition("lu");
    return hpx::util::report_errors();
}
