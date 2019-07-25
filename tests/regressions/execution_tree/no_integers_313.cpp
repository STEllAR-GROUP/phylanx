// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #313: PhySL Does Not Recognize Integer Numbers

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

int main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile("define(x, 2 * 2)", snippets);
    auto f = code.run();

    auto result = f();
    HPX_TEST_EQ(phylanx::execution_tree::extract_numeric_value(result)[0], 4.0);

    return hpx::util::report_errors();
}
