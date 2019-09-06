// Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// #406: shape() accepts anything for its second argument

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <string>
#include <utility>
#include <vector>

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

void test_shape_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

void test_shape_operation_fail(std::string const& code)
{
    bool caught_exception = false;
    try
    {
        compile_and_run(code);
        HPX_TEST(false);
    }
    catch (...)
    {
        caught_exception = true;
    }
    HPX_TEST(caught_exception);
}

///////////////////////////////////////////////////////////////////////////////
void test_shape()
{
    test_shape_operation("shape([1], 0)", "1");
    test_shape_operation("shape([[1], [2]], 0)", "2");
    test_shape_operation("shape([[1], [2]], 1)", "1");
}

void test_shape_fail()
{
    test_shape_operation_fail("shape([1], [0])");
    test_shape_operation_fail("shape([[1], [2]], [1])");
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_shape();
    test_shape_fail();

    return hpx::util::report_errors();
}
