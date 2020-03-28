//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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
void test_async_operation_immediate()
{
    std::string const code = R"(block(
            async(42)
        ))";

    auto a = compile_and_run(code);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_scalar_integer_value_strict(a()),
        std::int64_t(42));
}

void test_async_operation_immediate_concurrent()
{
    std::string const code_str = R"(
            list(async(42), async(43))
        )";

    auto code = compile_and_run(code_str);
    auto l = phylanx::execution_tree::extract_list_value(code());

    auto it = l.begin();
    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_integer_value(*it++),
        std::int64_t(42));
    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_integer_value(*it),
        std::int64_t(43));
}

void test_async_operation_immediate_concurrent_add()
{
    std::string const code_str = R"(
            async(42) + async(43)
        )";

    auto code = compile_and_run(code_str);

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_integer_value(code()),
        std::int64_t(85));
}

///////////////////////////////////////////////////////////////////////////////
void test_async_operation_variable()
{
    std::string const code = R"(block(
            define(a, async(42)), a
        ))";

    auto a = compile_and_run(code);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_scalar_integer_value_strict(a()),
        std::int64_t(42));
}

void test_async_operation_variable_concurrent()
{
    std::string const code_str = R"(block(
            define(a, async(42)),
            define(b, async(43)),
            list(a, b)
        ))";

    auto code = compile_and_run(code_str);
    auto l = phylanx::execution_tree::extract_list_value(code());

    auto it = l.begin();
    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_integer_value(*it++),
        std::int64_t(42));
    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_integer_value(*it),
        std::int64_t(43));
}

void test_async_operation_variable_concurrent_add()
{
    std::string const code_str = R"(block(
            define(a, async(42)),
            define(b, async(43)),
            a + b
        ))";

    auto code = compile_and_run(code_str);

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_integer_value(code()),
        std::int64_t(85));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_async_operation_immediate();
    test_async_operation_immediate_concurrent();
    test_async_operation_immediate_concurrent_add();

    test_async_operation_variable();
    test_async_operation_variable_concurrent();
    test_async_operation_variable_concurrent_add();

    return hpx::util::report_errors();
}
