// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #278: Python to PhySL Translation generates empty block #278

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

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

void test_parse_int_array()
{
    HPX_TEST_EQ(compile_and_run("[1, 2, 3, 4]"),
        compile_and_run("hstack(list(1, 2, 3, 4))"));

    HPX_TEST_NEQ(compile_and_run("[1., 2., 3., 4.]"),
        compile_and_run("hstack(list(1, 2, 3, 4))"));
    HPX_TEST_NEQ(compile_and_run("[1, 2, 3, 4.]"),
        compile_and_run("hstack(list(1, 2, 3, 4))"));
}

void test_parse_double_array()
{
    HPX_TEST_EQ(compile_and_run("[1., 2., 3., 4.]"),
        compile_and_run("hstack(list(1., 2., 3., 4.))"));

    HPX_TEST_NEQ(compile_and_run("[1, 2, 3, 4]"),
        compile_and_run("hstack(list(1., 2., 3., 4.))"));
}

int main(int argc, char* argv[])
{
    test_parse_int_array();
    test_parse_double_array();

    return hpx::util::report_errors();
}
