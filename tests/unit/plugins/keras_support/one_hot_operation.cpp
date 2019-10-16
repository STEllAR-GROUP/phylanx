// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <string>
#include <utility>

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
void test_one_hot_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_one_hot_operation("one_hot(2, 4)", "[0., 0., 1., 0.]");
    test_one_hot_operation("one_hot(42, 4)", "[0., 0., 0., 0.]");
    test_one_hot_operation("one_hot([1, 2, 3, 0, 3, 2], 4)",
        "[[ 0.,  1.,  0.,  0.],[ 0.,  0.,  1.,  0.], [ 0.,  0.,  0.,  1.], "
        "[1.,  0.,  0.,  0.], [ 0.,  0.,  0.,  1.],[ 0.,  0.,  1., 0.]]");
    test_one_hot_operation("one_hot([1, 2, 13, 0, 3, 2], 3)",
        "[[ 0.,  1.,  0.],[ 0.,  0.,  1.],[ 0.,  0.,  0.],[ 1.,  0.,  0.],"
        "[ 0.,  0.,  0.],[ 0.,  0.,  1.]]");
    test_one_hot_operation("one_hot([[0, 1, 2, 3],[3, 2, 1, 0]], 4)",
        "[[[ 1.,  0.,  0.,  0.],[ 0.,  1.,  0.,  0.],[ 0.,  0.,  1.,  0.], "
        "[0.,  0.,  0.,  1.]],[[ 0.,  0.,  0.,  1.],[ 0.,  0.,  1., 0.] "
        ",[ 0.,  1.,  0.,  0.],[ 1.,  0.,  0.,  0.]]]");

    return hpx::util::report_errors();
}
