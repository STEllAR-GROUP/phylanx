// Copyright (c) 2018 Adrian Serio
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// #792: Conversion of range to array by hstack requires a variable name

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

int main(int argc, char* argv[])
{
    // External Function
    char const* const codestr = "hstack(range(1, 4))";

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(
        "test", codestr, snippets, env);
    code.run();

    return hpx::util::report_errors();
}
