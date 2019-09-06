// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
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
void test_len_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_len_operation("len( list() )", "0");
    test_len_operation("len( list(1, 2) )", "2");
    test_len_operation("len( list(1, 2, 3) )", "3");
    test_len_operation(
        "len( \"Question of Life, Universe, and Everything\" )", "42");

    test_len_operation("len([1,2,3])", "3");
    test_len_operation("len([[1,2,3],[4,5,6]])", "2");

    test_len_operation("len([[[1,2,3],[4,5,6]],[[1,2,3],[4,5,6]]])", "2");
    test_len_operation("len([[[[1,2,3],[4,5,6]]],[[[1,2,3],[4,5,6]]]])", "2");

    return hpx::util::report_errors();
}
