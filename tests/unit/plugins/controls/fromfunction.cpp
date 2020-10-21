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
void test_fromfunction_operation(
    std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

//////////////////////////////////////////////////////////////////////////
void test_fromfunction0d()
{
    test_fromfunction_operation("fromfunction(lambda(nil), list())", "nil");
}

void test_fromfunction1d()
{
    test_fromfunction_operation(
        "fromfunction(lambda(i, list(i)), list(2))", "list([0., 1.])");

    test_fromfunction_operation(
        R"(fromfunction(lambda(i, list(i)), list(4), __arg(dtype, "int")))",
        "list([0, 1, 2, 3])");
    test_fromfunction_operation(
        R"(fromfunction(lambda(i, list(i)), list(3), __arg(dtype, "float")))",
        "list([0., 1., 2.])");
}

void test_fromfunction2d()
{
    test_fromfunction_operation(
        "fromfunction(lambda(i, j, list(i, j)), list(2, 3))",
        "list([[0., 0., 0.], [1., 1., 1.]], [[0., 1., 2.], [0., 1., 2.]])");

    test_fromfunction_operation(
        R"(fromfunction(
                lambda(i, j, list(i, j)), list(2, 3), __arg(dtype, "int"))
        )",
        "list([[0, 0, 0], [1, 1, 1]], [[0, 1, 2], [0, 1, 2]])");
    test_fromfunction_operation(
        R"(fromfunction(
                lambda(i, j, list(i, j)), list(2, 3), __arg(dtype, "float"))
        )",
        "list([[0., 0., 0.], [1., 1., 1.]], [[0., 1., 2.], [0., 1., 2.]])");
}

void test_fromfunction3d()
{
    test_fromfunction_operation(
        "fromfunction(lambda(i, j, k, list(i, j, k)), list(1, 2, 3))",
        "list([[[0., 0., 0.], [0., 0., 0.]]], [[[0., 0., 0.], [1., 1., 1.]]], "
             "[[[0., 1., 2.], [0., 1., 2.]]])");

    test_fromfunction_operation(
        R"(fromfunction(
                lambda(i, j, k, list(i, j, k)), list(1, 2, 3),
                __arg(dtype, "int"))
        )",
        "list([[[0, 0, 0], [0, 0, 0]]], [[[0, 0, 0], [1, 1, 1]]], [[[0, 1, 2], "
        "[0, 1, 2]]])");
    test_fromfunction_operation(
        R"(fromfunction(
                lambda(i, j, k, list(i, j, k)), list(1, 2, 3),
                __arg(dtype, "float"))
        )",
        "list([[[0., 0., 0.], [0., 0., 0.]]], [[[0., 0., 0.], [1., 1., 1.]]], "
        "[[[0., 1., 2.], [0., 1., 2.]]])");
}

void test_fromfunction4d()
{
    test_fromfunction_operation(
        "fromfunction(lambda(i, j, k, l, list(i, j, k, l)), list(1, 2, 3, 4))",
        R"(list([[[[0., 0., 0., 0.], [0., 0., 0., 0.], [0., 0., 0., 0.]],
                  [[0., 0., 0., 0.], [0., 0., 0., 0.], [0., 0., 0., 0.]]]],
                [[[[0., 0., 0., 0.], [0., 0., 0., 0.], [0., 0., 0., 0.]],
                  [[1., 1., 1., 1.], [1., 1., 1., 1.], [1., 1., 1., 1.]]]],
                [[[[0., 0., 0., 0.], [1., 1., 1., 1.], [2., 2., 2., 2.]],
                  [[0., 0., 0., 0.], [1., 1., 1., 1.], [2., 2., 2., 2.]]]],
                [[[[0., 1., 2., 3.], [0., 1., 2., 3.], [0., 1., 2., 3.]],
                  [[0., 1., 2., 3.], [0., 1., 2., 3.], [0., 1., 2., 3.]]]]))");

    test_fromfunction_operation(
        R"(fromfunction(
                lambda(i, j, k, l, list(i, j, k, l)), list(1, 2, 3, 4),
                __arg(dtype, "int"))
        )",
        R"(list([[[[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]],
                  [[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]]]],
                [[[[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]],
                  [[1, 1, 1, 1], [1, 1, 1, 1], [1, 1, 1, 1]]]],
                [[[[0, 0, 0, 0], [1, 1, 1, 1], [2, 2, 2, 2]],
                  [[0, 0, 0, 0], [1, 1, 1, 1], [2, 2, 2, 2]]]],
                [[[[0, 1, 2, 3], [0, 1, 2, 3], [0, 1, 2, 3]],
                  [[0, 1, 2, 3], [0, 1, 2, 3], [0, 1, 2, 3]]]]))");
    test_fromfunction_operation(
        R"(fromfunction(
                lambda(i, j, k, l, list(i, j, k, l)), list(1, 2, 3, 4),
                __arg(dtype, "float"))
        )",
        R"(list([[[[0., 0., 0., 0.], [0., 0., 0., 0.], [0., 0., 0., 0.]],
                  [[0., 0., 0., 0.], [0., 0., 0., 0.], [0., 0., 0., 0.]]]],
                [[[[0., 0., 0., 0.], [0., 0., 0., 0.], [0., 0., 0., 0.]],
                  [[1., 1., 1., 1.], [1., 1., 1., 1.], [1., 1., 1., 1.]]]],
                [[[[0., 0., 0., 0.], [1., 1., 1., 1.], [2., 2., 2., 2.]],
                  [[0., 0., 0., 0.], [1., 1., 1., 1.], [2., 2., 2., 2.]]]],
                [[[[0., 1., 2., 3.], [0., 1., 2., 3.], [0., 1., 2., 3.]],
                  [[0., 1., 2., 3.], [0., 1., 2., 3.], [0., 1., 2., 3.]]]]))");
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_fromfunction0d();
    test_fromfunction1d();
    test_fromfunction2d();
    test_fromfunction3d();
    test_fromfunction4d();

    return hpx::util::report_errors();
}
