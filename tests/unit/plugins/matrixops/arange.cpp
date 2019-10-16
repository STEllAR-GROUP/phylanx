// Copyright (c) 2017-2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
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

void test_arange(char const* code, char const* expectedstr)
{
    auto result = compile_and_run(code);
    auto expected = compile_and_run(expectedstr);

    HPX_TEST_EQ(result, expected);
}

int main(int argc, char* argv[])
{
    test_arange("arange(1)", "[0]");

    test_arange("arange(0, 5, 1)", "[0, 1, 2, 3, 4]");

    test_arange("arange(0.0, 5, 1)", "[0.0, 1.0, 2.0, 3.0, 4.0]");
    test_arange("arange(0, 5.0, 1)", "[0.0, 1.0, 2.0, 3.0, 4.0]");
    test_arange("arange(0, 5, 1.0)", "[0.0, 1.0, 2.0, 3.0, 4.0]");

    test_arange("arange(0, 5, 1.1)", "[0.0, 1.1, 2.2, 3.3, 4.4]");

    test_arange("arange(5.0, 0, -1.0)", "[5.0, 4.0, 3.0, 2.0, 1.0]");

    test_arange(
        R"(arange(0, 5, 1, __arg(dtype, "int")))", "[0, 1, 2, 3, 4]");
    test_arange(R"(arange(0, 5, 1, __arg(dtype, "float")))",
        "[0.0, 1.0, 2.0, 3.0, 4.0]");
    test_arange(R"(arange(0, 1, 1, __arg(dtype, "bool")))",
        "hstack(list(false))");

    test_arange(R"(arange(0.0, 5.0, 1.0, __arg(dtype, "int")))",
        "[0, 1, 2, 3, 4]");

    return hpx::util::report_errors();
}
