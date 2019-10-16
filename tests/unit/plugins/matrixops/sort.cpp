// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
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
void test_sort(std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_sort("sort([2., 1., 5., 4., -1.], 0, \"quicksort\")",
        "[-1., 1., 2., 4., 5.]");
    test_sort("sort([2., 1., 5., 4., -1.], -1, \"quicksort\")",
        "[-1., 1., 2., 4., 5.]");
    test_sort("sort([2., 1., 5., 4., -1.], nil, \"quicksort\")",
        "[-1., 1., 2., 4., 5.]");
    test_sort(
        "sort([[2., 1., 7., 4., 3.],[5., -4., 6., 9., 2.]], 0, \"quicksort\")",
        "[[2., -4., 6., 4., 2.], [5., 1., 7., 9., 3.]]");
    test_sort(
        "sort([[2., 1., 7., 4., 3.],[5., -4., 6., 9., 2.]], -2, \"quicksort\")",
        "[[2., -4., 6., 4., 2.], [5., 1., 7., 9., 3.]]");
    test_sort(
        "sort([[2., 1., 7., 4., 3.],[5., -4., 6., 9., 2.]], 1, \"quicksort\")",
        "[[1., 2., 3., 4., 7.], [-4., 2., 5., 6., 9.]]");
    test_sort(
        "sort([[2., 1., 7., 4., 3.],[5., -4., 6., 9., 2.]], -1, \"quicksort\")",
        "[[1., 2., 3., 4., 7.], [-4., 2., 5., 6., 9.]]");
    test_sort("sort([[2., 1., 7., 4., 3.],[5., -4., 6., 9., 2.]], nil, "
              "\"quicksort\")",
        "[-4., 1., 2., 2., 3., 4., 5., 6., 7., 9.]");

    test_sort(
        "sort([[[2., 1., 7., 4., 3.],[5., -24., 6., 9., 22.]], [[12., -8., "
        "17., 14., 1.],[15., -14., 16., 19., 12.]]], 0, \"quicksort\")",
        "[[[2., -8., 7., 4., 1.], [5., -24., 6., 9., 12.]], [[12., 1., 17., "
        "14., 3.], [15., -14., 16., 19., 22.]]]");
    test_sort(
        "sort([[[2., 1., 7., 4., 3.],[5., -24., 6., 9., 22.]], [[12., -8., "
        "17., 14., 1.],[15., -14., 16., 19., 12.]]], -3, \"quicksort\")",
        "[[[2., -8., 7., 4., 1.], [5., -24., 6., 9., 12.]], [[12., 1., 17., "
        "14., 3.], [15., -14., 16., 19., 22.]]]");
    test_sort(
        "sort([[[2., 1., 7., 4., 3.],[5., -24., 6., 9., 22.]], [[12., -8., "
        "17., 14., 1.],[15., -14., 16., 19., 12.]]], 1, \"quicksort\")",
        "[[[2., -24., 6., 4., 3.], [5., 1., 7., 9., 22.]], [[12., -14., 16., "
        "14., 1.], [15., -8., 17., 19., 12.]]]");
    test_sort(
        "sort([[[2., 1., 7., 4., 3.],[5., -24., 6., 9., 22.]], [[12., -8., "
        "17., 14., 1.],[15., -14., 16., 19., 12.]]], -2, \"quicksort\")",
        "[[[2., -24., 6., 4., 3.], [5., 1., 7., 9., 22.]], [[12., -14., 16., "
        "14., 1.], [15., -8., 17., 19., 12.]]]");
    test_sort(
        "sort([[[2., 1., 7., 4., 3.],[5., -24., 6., 9., 22.]], [[12., -8., "
        "17., 14., 1.],[15., -14., 16., 19., 12.]]], 2, \"quicksort\")",
        "[[[1., 2., 3., 4., 7.], [-24., 5., 6., 9., 22.]], [[-8., 1., 12., "
        "14., 17.], [-14., 12., 15., 16., 19.]]]");
    test_sort(
        "sort([[[2., 1., 7., 4., 3.],[5., -24., 6., 9., 22.]], [[12., -8., "
        "17., 14., 1.],[15., -14., 16., 19., 12.]]], -1, \"quicksort\")",
        "[[[1., 2., 3., 4., 7.], [-24., 5., 6., 9., 22.]], [[-8., 1., 12., "
        "14., 17.], [-14., 12., 15., 16., 19.]]]");
    test_sort(
        "sort([[[2., 1., 7., 4., 3.],[5., -24., 6., 9., 22.]], [[12., -8., "
        "17., 14., 1.],[15., -14., 16., 19., 12.]]], nil, \"quicksort\")",
        "[-24., -14., -8., 1., 1., 2., 3., 4., 5., 6., 7., 9., 12., 12., 14., "
        "15., 16., 17., 19., 22.]");

    return hpx::util::report_errors();
}
