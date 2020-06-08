// Copyright (c) 2020 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/modules/testing.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

void test_operation(char const* code, char const* expectedstr)
{
    auto result = compile_and_run(code);
    auto expected = compile_and_run(expectedstr);

    HPX_TEST_EQ(result, expected);
}

int main(int argc, char* argv[])
{
    test_operation(R"(all([[1, 0, -1], [1, 2, 0]]))", "false");
    test_operation(
        R"(all([[1, 0, -1], [1, 2, 0]], 0))", "[true, false, false]");

    return hpx::util::report_errors();
}
