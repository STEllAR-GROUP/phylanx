// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/plugins/listops/dictionary_operation.hpp>
#include <phylanx/plugins/listops/dict_keys_operation.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

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
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_dict_keys_operation(std::string const& code)
{
    compile_and_run(code);

    //auto temp =
    //    phylanx::execution_tree::extract_list_value(compile_and_run(code));
        //phylanx::execution_tree::extract_list_value(compile_and_run(code));

    //for (phylanx::ir::range_iterator it = temp.begin(); it != temp.end(); ++it)
    //{
    //    std::cout << *it << std::endl;
    //}
    //auto dict_keys_list = phylanx::execution_tree::extract_value();
    //auto dict_keys_list_temp =
    //    phylanx::execution_tree::extract_list_value(dict_keys_list);
    //for (phylanx::ir::range_iterator it = dict_keys_list_temp.begin();
    //     it != dict_keys_list_temp.end();
    //     it++)
    //{
    //    std::cout << *it << std::endl;
    //}
}

void test_dict_keys_empty_operation(std::string const& code)
{
    bool exception_thrown = false;
    try
    {
        // Must throw an exception
        compile_and_run(code);
        HPX_TEST(false);
    }
    catch (std::exception const&)
    {
        exception_thrown = true;
    }
    HPX_TEST(exception_thrown);
}

void test_dict_keys_empt_arg_operation(std::string const& code)
{
    bool exception_thrown = false;
    try
    {
        // Must throw an exception
        compile_and_run(code);
        HPX_TEST(false);
    }
    catch (std::exception const&)
    {
        exception_thrown = true;
    }
    HPX_TEST(exception_thrown);
}

int main(int argc, char* argv[])
{
    test_dict_keys_operation("dict_keys(dict(list(list(1, 2), list(3, 4))))");
    test_dict_keys_empty_operation("dict_keys()");
    test_dict_keys_empt_arg_operation("dict_keys(dict())");

    return hpx::util::report_errors();
}
