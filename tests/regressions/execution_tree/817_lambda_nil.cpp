// Copyright (c) 2019 R. Tohid
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// #817: PhySL: lambda returning nil

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

int main(int argc, char* argv[])
{
    // External Function
    char const* const codestr = R"(
        define(test, lambda(nil))
        test()
    )";

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(
        "test", codestr, snippets, env);
    auto test = code.run();

    test();

    return hpx::util::report_errors();
}
