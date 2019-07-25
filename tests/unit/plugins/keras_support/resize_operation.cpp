// Copyright (c) 2019 Shahrzad Shirzad
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
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_resize_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_resize_operation("resize_images([[1., 2., 3.], [6., 1., 9.0]], "
                          "2, 4,\"bilinear\")",
        "[[1., 1.25, 1.5, 1.75, 2., 2.25, 2.5, 2.75, 3., 3., 3., 3], [3.5, 3., "
        "2.5, 2., 1.5, 2.625, 3.75, 4.875, 6., 6, 6., 6.], [6., 4.75, 3.5, "
        "2.25, 1., 3., 5., 7., 9., 9., 9., 9.], [6., 4.75, 3.5, 2.25, 1., 3., "
        "5., 7., 9., 9., 9., 9.]]");
    test_resize_operation(
        "resize_images([[1., 2., 3., -4.], [7., 8., 9., 2.]], "
        "3, 4,\"bilinear\")",
        "[[1., 1.25, 1.5, 1.75, 2., 2.25, 2.5, 2.75, 3., 1.25, -0.5, -2.25, "
        "-4., -4., -4., -4], [3., 3.25, 3.5, 3.75, 4., 4.25, 4.5, 4.75, 5., "
        "3.25, 1.5, -0.25, -2., -2., -2., -2], [5, 5.25, 5.5, 5.75, 6., 6.25, "
        "6.5, 6.75, 7, 5.25, 3.5, 1.75, 0, 0, 0, 0], [7., 7.25, 7.5, 7.75, 8., "
        "8.25, 8.5, 8.75, 9., 7.25, 5.5, 3.75, 2., 2., 2., 2.], [7., 7.25, "
        "7.5, 7.75, 8., 8.25, 8.5, 8.75, 9., 7.25, 5.5, 3.75, 2., 2., 2., 2.], "
        "[7., 7.25, 7.5, 7.75, 8., 8.25, 8.5, 8.75, 9., 7.25, 5.5, 3.75, 2., "
        "2., 2., 2.]]");
    test_resize_operation("resize_images([[1., 2., 3.], [6., 1., 9.0]], "
                          "2, 4,\"nearest\")",
        "[[1., 1., 1., 1., 2., 2., 2., 2., 3., 3., 3., 3.], [1., 1., 1., 1., "
        "2., 2., 2., 2., 3., 3., 3., 3.], [6., 6., 6., 6., 1., 1., 1., 1., 9., "
        "9., 9., 9.], [6., 6., 6., 6., 1., 1., 1., 1., 9., 9., 9., 9.]]");
    test_resize_operation(
        "resize_images([[1., 2., 3., -4.], [7., 8., 9., 2.]], "
        "3, 4,\"nearest\")",
        "[[1., 1., 1., 1., 2., 2., 2., 2., 3., 3., 3., 3., -4., -4., -4., "
        "-4.], [1., 1., 1., 1., 2., 2., 2., 2., 3., 3., 3., 3., -4., -4., -4., "
        "-4.], [1., 1., 1., 1., 2., 2., 2., 2., 3., 3., 3., 3., -4., -4., -4., "
        "-4.], [7., 7., 7., 7., 8., 8., 8., 8., 9., 9., 9., 9., 2., 2., 2., "
        "2.], [7., 7., 7., 7., 8., 8., 8., 8., 9., 9., 9., 9., 2., 2., 2., "
        "2.], [7., 7., 7., 7., 8., 8., 8., 8., 9., 9., 9., 9., 2., 2., 2., "
        "2.]]");

    return hpx::util::report_errors();
}
