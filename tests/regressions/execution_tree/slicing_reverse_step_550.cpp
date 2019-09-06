// Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// #406: slicing() accepts anything for its second argument

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

void test_slicing_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
void test_slicing()
{
    std::string const code = R"(
        define(test, x, block(
            define(result, slice(x, list(nil, nil, -(1)))),
            result
        ))
        test(hstack(list(1, 2, 3, 4, 5)))
    )";

    test_slicing_operation(code, "hstack(list(5, 4, 3, 2, 1))");
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_slicing();

    return hpx::util::report_errors();
}
