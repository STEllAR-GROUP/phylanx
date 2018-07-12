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
// <image url="$(ItemDir)/images/test_filter_operation_lambda.dot.png" />
//
void test_filter_operation_lambda()
{
    std::string const code = R"(
            filter(lambda(x, x > 1), list(1, 2, 3))
        )";

    auto result = phylanx::execution_tree::extract_list_value(compile(code)());
    HPX_TEST_EQ(result.size(), 2ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 3.0);
}

///////////////////////////////////////////////////////////////////////////////
// <image url="$(ItemDir)/images/test_filter_operation_func.dot.png" />
//
void test_filter_operation_func()
{
    std::string const code = R"(block(
            define(f, x, x > 1),
            filter(f, list(1, 2, 3))
        ))";

    auto result = phylanx::execution_tree::extract_list_value(compile(code)());

    HPX_TEST_EQ(result.size(), 2ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 3.0);
}

///////////////////////////////////////////////////////////////////////////////
// <image url="$(ItemDir)/images/test_filter_operation_func_lambda.dot.png" />
//
void test_filter_operation_func_lambda()
{
    std::string const code = R"(block(
            define(f, lambda(x, x > 1)),
            filter(f, list(1, 2, 3))
        ))";

    auto result = phylanx::execution_tree::extract_list_value(compile(code)());

    HPX_TEST_EQ(result.size(), 2ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 3.0);
}

///////////////////////////////////////////////////////////////////////////////
// <image url="$(ItemDir)/images/test_filter_operation_lambda_arg.dot.png" />
//
void test_filter_operation_lambda_arg()
{
    std::string const code_str = R"(block(
            define(f, a, filter(lambda(x, x > a), list(1, 2, 3))),
            f
        ))";

    auto code = compile(code_str);
    auto result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{42}));

    HPX_TEST_EQ(result.size(), 0ul);

    result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{2}));

    HPX_TEST_EQ(result.size(), 1ul);

    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*result.begin())[0], 3.0);
}

///////////////////////////////////////////////////////////////////////////////
// <image url="$(ItemDir)/images/test_filter_operation_func_arg.dot.png" />
//
void test_filter_operation_func_arg()
{
    std::string const code_str = R"(block(
            define(f, a, block(
                define(fmap, x, x > a),
                filter(fmap, list(1, 2, 3))
            )),
            f
        ))";

    auto code = compile(code_str);
    auto result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{42}));

    HPX_TEST_EQ(result.size(), 0ul);

    result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{2}));

    HPX_TEST_EQ(result.size(), 1ul);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*result.begin())[0], 3.0);
}

///////////////////////////////////////////////////////////////////////////////
// <image url="$(ItemDir)/images/test_filter_operation_func_lambda_arg.dot.png" />
//
void test_filter_operation_func_lambda_arg()
{
    std::string const code_str = R"(block(
            define(f, a, block(
                define(fmap, lambda(x, x > a)),
                filter(fmap, list(1, 2, 3))
            )),
            f
        ))";

    auto code = compile(code_str);
    auto result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{42}));

    HPX_TEST_EQ(result.size(), 0ul);

    result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{2}));

    HPX_TEST_EQ(result.size(), 1ul);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*result.begin())[0], 3.0);
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_filter_operation_lambda();
    test_filter_operation_func();
    test_filter_operation_func_lambda();

    test_filter_operation_lambda_arg();
    test_filter_operation_func_arg();
    test_filter_operation_func_lambda_arg();

    return hpx::util::report_errors();
}
