// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <cstdint>
#include <string>

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
void test_indices_operation(
    std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

//////////////////////////////////////////////////////////////////////////
void test_indices0d()
{
    test_indices_operation("indices(list())", "list()");
    test_indices_operation("indices(list(), __arg(sparse, true))", "list()");
    test_indices_operation("indices(list(), __arg(sparse, false))", "list()");
}

void test_indices1d()
{
    test_indices_operation("indices(list(2))", "list([0, 1])");

    test_indices_operation(
        "indices(list(2), __arg(sparse, true))", "list([0, 1])");
    test_indices_operation(
        "indices(list(2), __arg(sparse, false))", "list([0, 1])");

    test_indices_operation(
        R"(indices(list(4), __arg(dtype, "int"), __arg(sparse, true)))",
        "list([0, 1, 2, 3])");
    test_indices_operation(
        R"(indices(list(3), __arg(dtype, "float"), __arg(sparse, false)))",
        "list([0., 1., 2.])");
}

void test_indices2d()
{
    test_indices_operation("indices(list(2, 3))",
        "list([[0, 0, 0], [1, 1, 1]], [[0, 1, 2], [0, 1, 2]])");

    test_indices_operation("indices(list(2, 3), __arg(sparse, false))",
        "list([[0, 0, 0], [1, 1, 1]], [[0, 1, 2], [0, 1, 2]])");
    test_indices_operation("indices(list(2, 3), __arg(sparse, true))",
        "list([[0], [1]], [[0, 1, 2]])");

    test_indices_operation(
        R"(indices(list(2, 3), __arg(dtype, "int"), __arg(sparse, false)))",
        "list([[0, 0, 0], [1, 1, 1]], [[0, 1, 2], [0, 1, 2]])");
    test_indices_operation(
        R"(indices(list(2, 3), __arg(dtype, "float"), __arg(sparse, true)))",
        "list([[0.], [1.]], [[0., 1., 2.]])");
}

void test_indices3d()
{
    test_indices_operation("indices(list(1, 2, 3))",
        "list([[[0, 0, 0], [0, 0, 0]]], [[[0, 0, 0], [1, 1, 1]]], [[[0, 1, 2], "
        "[0, 1, 2]]])");

    test_indices_operation("indices(list(1, 2, 3), __arg(sparse, false))",
        "list([[[0, 0, 0], [0, 0, 0]]], [[[0, 0, 0], [1, 1, 1]]], [[[0, 1, 2], "
        "[0, 1, 2]]])");
    test_indices_operation("indices(list(1, 2, 3), __arg(sparse, true))",
        "list([[[0]]], [[[0], [1]]], [[[0, 1, 2]]])");

    test_indices_operation(
        R"(indices(list(1, 2, 3), __arg(dtype, "int"), __arg(sparse, false)))",
        "list([[[0, 0, 0], [0, 0, 0]]], [[[0, 0, 0], [1, 1, 1]]], [[[0, 1, 2], "
        "[0, 1, 2]]])");
    test_indices_operation(
        R"(indices(list(1, 2, 3), __arg(dtype, "float"), __arg(sparse, true)))",
        "list([[[0.]]], [[[0.], [1.]]], [[[0., 1., 2.]]])");
}

void test_indices4d()
{
    test_indices_operation("indices(list(1, 2, 3, 4))",
        R"(list([[[[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]],
                  [[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]]]],
                [[[[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]],
                  [[1, 1, 1, 1], [1, 1, 1, 1], [1, 1, 1, 1]]]],
                [[[[0, 0, 0, 0], [1, 1, 1, 1], [2, 2, 2, 2]],
                  [[0, 0, 0, 0], [1, 1, 1, 1], [2, 2, 2, 2]]]],
                [[[[0, 1, 2, 3], [0, 1, 2, 3], [0, 1, 2, 3]],
                  [[0, 1, 2, 3], [0, 1, 2, 3], [0, 1, 2, 3]]]]))");

    test_indices_operation("indices(list(1, 2, 3, 4), __arg(sparse, false))",
        R"(list([[[[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]],
                  [[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]]]],
                [[[[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]],
                  [[1, 1, 1, 1], [1, 1, 1, 1], [1, 1, 1, 1]]]],
                [[[[0, 0, 0, 0], [1, 1, 1, 1], [2, 2, 2, 2]],
                  [[0, 0, 0, 0], [1, 1, 1, 1], [2, 2, 2, 2]]]],
                [[[[0, 1, 2, 3], [0, 1, 2, 3], [0, 1, 2, 3]],
                  [[0, 1, 2, 3], [0, 1, 2, 3], [0, 1, 2, 3]]]]))");
    test_indices_operation("indices(list(1, 2, 3, 4), __arg(sparse, true))",
        R"(list([[[[0]]]],
                [[[[0]], [[1]]]],
                [[[[0], [1], [2]]]],
                [[[[0, 1, 2, 3]]]]))");

    test_indices_operation(
        R"(indices(list(1, 2, 3, 4), __arg(dtype, "int"), __arg(sparse, false)))",
        R"(list([[[[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]],
                  [[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]]]],
                [[[[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]],
                  [[1, 1, 1, 1], [1, 1, 1, 1], [1, 1, 1, 1]]]],
                [[[[0, 0, 0, 0], [1, 1, 1, 1], [2, 2, 2, 2]],
                  [[0, 0, 0, 0], [1, 1, 1, 1], [2, 2, 2, 2]]]],
                [[[[0, 1, 2, 3], [0, 1, 2, 3], [0, 1, 2, 3]],
                  [[0, 1, 2, 3], [0, 1, 2, 3], [0, 1, 2, 3]]]]))");
    test_indices_operation(
        R"(indices(list(1, 2, 3, 4), __arg(dtype, "float"), __arg(sparse, true)))",
        R"(list([[[[0.]]]],
                [[[[0.]], [[1.]]]],
                [[[[0.], [1.], [2.]]]],
                [[[[0., 1., 2., 3.]]]]))");
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_indices0d();
    test_indices1d();
    test_indices2d();
    test_indices3d();
    test_indices4d();

    return hpx::util::report_errors();
}
