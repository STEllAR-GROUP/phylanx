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
void test_fmap_operation_lambda()
{
    std::string const code = R"(
            fmap(lambda(x, x + 1), list(1, 2, 3))
        )";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 3.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 4.0);
}

void test_fmap_operation_func()
{
    std::string const code = R"(block(
            define(f, x, x + 1),
            fmap(f, list(1, 2, 3))
        ))";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 3.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 4.0);
}

void test_fmap_operation_func_lambda()
{
    std::string const code = R"(block(
            define(f, lambda(x, x + 1)),
            fmap(f, list(1, 2, 3))
        ))";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 3.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 4.0);
}

///////////////////////////////////////////////////////////////////////////////
void test_fmap_operation_lambda_arg()
{
    std::string const code_str = R"(block(
            define(f, a, fmap(lambda(x, x + a), list(1, 2, 3))),
            f
        ))";

    auto code = compile_and_run(code_str);
    auto result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{42}));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 43.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 44.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 45.0);

    result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{2}));

    it = result.begin();
    HPX_TEST_EQ(result.size(), 3ul);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 3.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 4.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 5.0);
}

void test_fmap_operation_func_arg()
{
    std::string const code_str = R"(block(
            define(f, a, block(
                define(ffmap, x, x + a),
                fmap(ffmap, list(1, 2, 3))
            )),
            f
        ))";

    auto code = compile_and_run(code_str);
    auto result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{42}));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 43.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 44.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 45.0);

    result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{2}));

    HPX_TEST_EQ(result.size(), 3ul);

    it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 3.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 4.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 5.0);
}

/// <image url="$(ItemDir)/images/test_fmap_operation_func_lambda_arg.dot.png" />
//
void test_fmap_operation_func_lambda_arg()
{
    std::string const code_str = R"(block(
            define(f, a, block(
                define(ffmap, lambda(x, x + a)),
                fmap(ffmap, list(1, 2, 3))
            )),
            f
        ))";

    auto code = compile_and_run(code_str);
    auto result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{42}));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 43.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 44.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 45.0);

    result = phylanx::execution_tree::extract_list_value(
        code(std::int64_t{2}));

    it = result.begin();
    HPX_TEST_EQ(result.size(), 3ul);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 3.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 4.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 5.0);
}

///////////////////////////////////////////////////////////////////////////////
void test_fmap_operation_lambda2()
{
    std::string const code = R"(
            fmap(lambda(x, y, x + y), list(1, 2, 3), list(1, 2, 3))
        )";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 4.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 6.0);
}

void test_fmap_operation_builtin2()
{
    std::string const code = R"(
            fmap(__add, list(1, 2, 3), list(1, 2, 3))
        )";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 4.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 6.0);
}

void test_fmap_operation_func2()
{
    std::string const code = R"(block(
            define(f, x, y, x + y),
            fmap(f, list(1, 2, 3), list(1, 2, 3))
        ))";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 4.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 6.0);
}

void test_fmap_operation_func_lambda2()
{
    std::string const code = R"(block(
            define(f, lambda(x, y, x + y)),
            fmap(f, list(1, 2, 3), list(1, 2, 3))
        ))";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 3ul);

    auto it = result.begin();
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 2.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it++)[0], 4.0);
    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(*it)[0], 6.0);
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_fmap_operation_lambda();
    test_fmap_operation_func();
    test_fmap_operation_func_lambda();

    test_fmap_operation_lambda_arg();
    test_fmap_operation_func_arg();
    test_fmap_operation_func_lambda_arg();

    test_fmap_operation_lambda2();
    test_fmap_operation_builtin2();
    test_fmap_operation_func2();
    test_fmap_operation_func_lambda2();

    return hpx::util::report_errors();
}
