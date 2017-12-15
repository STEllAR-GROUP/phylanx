//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #131: Pattern matching is not working for negative values

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>

std::string const code = R"(block(
    define(x, y,
        slice(y, 0, 569, -1, 0)
    ),
    x
))";

int main(int argc, char* argv[])
{
    try
    {
        phylanx::execution_tree::compiler::function_list snippets;
        phylanx::execution_tree::compile(code, snippets);
    }
    catch(...)
    {
        HPX_TEST(false);        // shouldn't throw any exceptions
    }

    return hpx::util::report_errors();
}
