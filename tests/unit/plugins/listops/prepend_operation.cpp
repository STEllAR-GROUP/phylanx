// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>

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

///////////////////////////////////////////////////////////////////////////////
void test_prepend_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_prepend_operation("prepend( 3, list(1, 42) )", "list(3, 1, 42)");

    test_prepend_operation("prepend( list(1, 42), list(3, 4) )",
        "list(list(1, 42), 3, 4)");
    test_prepend_operation("prepend( list(), list() )", "list(list())");
    test_prepend_operation(
        "prepend( list(1, 42), list() )", "list(list(1, 42))");
    test_prepend_operation(
        "prepend( list(), list(1, 42) )", "list(list(), 1, 42)");

    return hpx::util::report_errors();
}
