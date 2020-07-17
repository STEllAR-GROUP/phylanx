//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #509: Empty list defined inside Phylanx function has an issue

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/distributed/iostream.hpp>
#include <hpx/modules/testing.hpp>

#include <string>
#include <sstream>

std::string const hstack = "block(define(f, a, hstack(list(a, a))), f)";
std::string const vstack = "block(define(f, a, vstack(list(a, a))), f)";
std::string const dstack = "block(define(f, a, dstack(list(a, a))), f)";

int main(int argc, char* argv[])
{
    auto arg = phylanx::ir::node_data<double>{41.0};

    phylanx::execution_tree::compiler::function_list snippets;
    auto const& hstack_f_code =
        phylanx::execution_tree::compile(hstack, snippets);
    auto hstack_f = hstack_f_code.run();

    auto hstack_data =
        phylanx::execution_tree::extract_numeric_value(hstack_f(arg));

    HPX_TEST(hstack_data.num_dimensions() == 1);
    HPX_TEST(hstack_data.dimension(0) == 2);
    HPX_TEST(hstack_data.size() == 2);

    auto const& vstack_f_code =
        phylanx::execution_tree::compile(vstack, snippets);
    auto vstack_f = vstack_f_code.run();

    auto vstack_data =
        phylanx::execution_tree::extract_numeric_value(vstack_f(arg));

    HPX_TEST(vstack_data.num_dimensions() == 2);
    HPX_TEST(vstack_data.dimension(0) == 2);
    HPX_TEST(vstack_data.dimension(1) == 1);
    HPX_TEST(vstack_data.size() == 2);

    auto const& dstack_f_code =
        phylanx::execution_tree::compile(dstack, snippets);
    auto dstack_f = dstack_f_code.run();

    auto dstack_data =
        phylanx::execution_tree::extract_numeric_value(dstack_f(arg));

    HPX_TEST(dstack_data.num_dimensions() == 3);
    HPX_TEST(dstack_data.dimension(0) == 2);
    HPX_TEST(dstack_data.dimension(1) == 1);
    HPX_TEST(dstack_data.dimension(2) == 1);
    HPX_TEST(dstack_data.size() == 2);

    return hpx::util::report_errors();
}
