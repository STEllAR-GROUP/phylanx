//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& code)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    return phylanx::execution_tree::compile(code, snippets, env);
}

///////////////////////////////////////////////////////////////////////////////
void test_for_each_operation_lambda()
{
    std::string const code = R"(block(
            define(x, 0),
            for_each(lambda(y, store(x, y + x)), make_list(1, 2, 3)),
            x
        ))";

    auto result = phylanx::execution_tree::extract_numeric_value(compile(code)());

    HPX_TEST_EQ(result[0], 6);
}

void test_for_each_operation_func()
{
    std::string const code = R"(block(
            define(x, 0),
            define(f, y, store(x, y + x)),
            for_each(f, make_list(1, 2, 3)),
            x
        ))";

    auto result = phylanx::execution_tree::extract_numeric_value(compile(code)());

    HPX_TEST_EQ(result[0], 6);
}

void test_for_each_operation_func_lambda()
{
    std::string const code = R"(block(
            define(x, 0),
            define(f, lambda(y, store(x, y + x))),
            for_each(f, make_list(1, 2, 3)),
            x
        ))";

    auto result = phylanx::execution_tree::extract_numeric_value(compile(code)());

    HPX_TEST_EQ(result[0], 6);
}

///////////////////////////////////////////////////////////////////////////////
void test_for_each_operation_lambda_arg()
{
    std::string const code_str = R"(block(
            define(f, a, block(
                define(x, 0),
                for_each(lambda(y, store(x, y + x + a)), make_list(1, 2, 3)),
                x
            )),
            f
        ))";

    {
        auto code = compile(code_str);
        auto result = phylanx::execution_tree::extract_numeric_value(
            code(std::int64_t{42}));
        HPX_TEST_EQ(result[0], 132);
    }
    {
        auto code = compile(code_str);
        auto result = phylanx::execution_tree::extract_numeric_value(
            code(std::int64_t{2}));
        HPX_TEST_EQ(result[0], 12);
    }
}

void test_for_each_operation_func_arg()
{
    std::string const code_str = R"(block(
            define(f, a, block(
                define(x, 0),
                define(ffor_each, y, store(x, y + x + a)),
                for_each(ffor_each, make_list(1, 2, 3)),
                x
            )),
            f
        ))";

    {
        auto code = compile(code_str);
        auto result = phylanx::execution_tree::extract_numeric_value(
            code(std::int64_t{42}));
        HPX_TEST_EQ(result[0], 132);
    }
    {
        auto code = compile(code_str);
        auto result = phylanx::execution_tree::extract_numeric_value(
            code(std::int64_t{2}));
        HPX_TEST_EQ(result[0], 12);
    }
}

void test_for_each_operation_func_lambda_arg()
{
    std::string const code_str = R"(block(
            define(f, a, block(
                define(x, 0),
                define(ffor_each, lambda(y, store(x, y + x + a))),
                for_each(ffor_each, make_list(1, 2, 3)),
                x
            )),
            f
        ))";

    {
        auto code = compile(code_str);
        auto result = phylanx::execution_tree::extract_numeric_value(
            code(std::int64_t{42}));
        HPX_TEST_EQ(result[0], 132);
    }
    {
        auto code = compile(code_str);
        auto result = phylanx::execution_tree::extract_numeric_value(
            code(std::int64_t{2}));
        HPX_TEST_EQ(result[0], 12);
    }
}

void test_for_each_break()
{
    std::string const code = R"(block(
            define(x, 0),
            for_each(
                lambda(y, block(store(x, y), if(y < 2, false, true))),
                make_list(1, 2, 3)
            ),
            x
        ))";

    auto result = phylanx::execution_tree::extract_numeric_value(compile(code)());

    HPX_TEST_EQ(result[0], 2);
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_for_each_operation_lambda();
    test_for_each_operation_func();
    test_for_each_operation_func_lambda();

    test_for_each_operation_lambda_arg();
    test_for_each_operation_func_arg();
    test_for_each_operation_func_lambda_arg();

    test_for_each_break();

    return hpx::util::report_errors();
}
