// Copyright (c) 2018 Bibek Wagle
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #413: Last PhySL line cannot be a comment

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>

std::string const code1 = R"(
    0 + 0
    // trailing comment
)";

std::string const code2 = R"(
    0 + 0
    // trailing comment)";

int main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compile(code1, snippets);
    phylanx::execution_tree::compile(code2, snippets);

    return hpx::util::report_errors();
}
