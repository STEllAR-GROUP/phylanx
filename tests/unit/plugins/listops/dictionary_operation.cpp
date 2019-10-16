// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/plugins/listops/dictionary_operation.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

//////////////////////////////////////////////////////////////////////////
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
void test_dictionary_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

//////////////////////////////////////////////////////////////////////////

void test_dict_operation()
{
    char const* const code_1 =
        "dict(list(list(42, \"Question of Life, Universe, and Everything\"), "
        "list(\"Question?\", 42.0)))";

    char const* const code_2 =
        "dict(list(list(42, \"Question of Life, Universe, and Everything\"), "
        "list(\"Question?\", 42.0)))";

    HPX_TEST_EQ(compile_and_run(code_1), compile_and_run(code_2));
}

void test_dict_key()
{
    char const* const code = "list(list(42, \"Question of Life, Universe, and "
                             "Everything\"), list(\"Question?\", 42.0))";

    phylanx::ir::range key_value =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    phylanx::execution_tree::primitive dict =
        phylanx::execution_tree::primitives::create_dict_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(phylanx::execution_tree::primitive_argument_type{
                    phylanx::ir::range(key_value)})});

    phylanx::execution_tree::primitive_argument_type temp = dict.eval().get();

    phylanx::ir::dictionary f =
        phylanx::execution_tree::extract_dictionary_value(temp);

    HPX_TEST_EQ(f[phylanx::execution_tree::primitive_argument_type{
                    phylanx::ir::node_data<std::int64_t>(42)}],
        phylanx::util::recursive_wrapper<
            phylanx::execution_tree::primitive_argument_type>{
            std::string("Question of Life, Universe, and Everything")});

    HPX_TEST_EQ(f[phylanx::util::recursive_wrapper<
                    phylanx::execution_tree::primitive_argument_type>{
                    std::string("Question?")}],
        phylanx::util::recursive_wrapper<
            phylanx::execution_tree::primitive_argument_type>{
            phylanx::ir::node_data<double>(42.0)});
}

void test_dict_empty_operation(std::string const& code)
{
    phylanx::ir::dictionary dict;
    HPX_TEST_EQ(
        phylanx::execution_tree::primitive_argument_type{std::move(dict)},
        compile_and_run(code));
}

int main(int argc, char* argv[])
{
    test_dict_operation();
    test_dict_key();

    test_dict_empty_operation("dict(list())");
    test_dict_empty_operation("dict()");

    return hpx::util::report_errors();
}
