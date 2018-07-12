//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #509: Empty list defined inside Phylanx function has an issue

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <sstream>

std::string const hstack = "block(hstack())";
std::string const vstack = "block(vstack())";

int main(int argc, char* argv[])
{
    auto arg = phylanx::ir::node_data<double>{41.0};

    phylanx::execution_tree::compiler::function_list snippets;
    auto hstack_f_code = phylanx::execution_tree::compile(hstack, snippets);
    auto hstack_f = hstack_f_code.run();

    auto hstack_data =
        phylanx::execution_tree::extract_numeric_value(hstack_f(arg, arg));

    HPX_TEST(hstack_data.num_dimensions() == 1);
    HPX_TEST(hstack_data.dimension(0) == 0);
    HPX_TEST(hstack_data.size() == 0);

    auto vstack_f_code = phylanx::execution_tree::compile(vstack, snippets);
    auto vstack_f = vstack_f_code.run();

    auto vstack_data =
        phylanx::execution_tree::extract_numeric_value(vstack_f(arg, arg));

    HPX_TEST(vstack_data.num_dimensions() == 2);
    HPX_TEST(vstack_data.dimension(0) == 0);
    HPX_TEST(vstack_data.dimension(1) == 1);
    HPX_TEST(vstack_data.size() == 0);

    return hpx::util::report_errors();
}
