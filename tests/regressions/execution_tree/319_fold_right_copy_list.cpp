// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #278: Python to PhySL Translation generates empty block #278

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run();
}

void test_fold_right_copy_list()
{
    std::string const code = R"(block(
            define(copy_list, l, fold_right(hstack, list(), l)),
            copy_list(list(1, 2, 3, 4))
        ))";

    auto result = phylanx::execution_tree::extract_integer_value_strict(
        compile_and_run(code));

    std::string const expected_str = R"(
            hstack(list(1, 2, 3, 4))
        )";

    auto expected_result =
        phylanx::execution_tree::extract_integer_value_strict(
            compile_and_run(expected_str));

    HPX_TEST_EQ(result, expected_result);
}

int main(int argc, char* argv[])
{
    test_fold_right_copy_list();

    return hpx::util::report_errors();
}
