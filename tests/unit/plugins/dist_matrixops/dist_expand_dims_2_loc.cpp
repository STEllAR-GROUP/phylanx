// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& name, std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code =
        phylanx::execution_tree::compile(name, codestr, snippets, env);
    return code.run().arg_;
}

void test_expand_dims_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_expand_dims_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_0", R"(
            expand_dims(
                annotate_d([1, 2, 3, 4], "array_0",
                list("tile", list("columns", 2, 6)))
            , 0)
        )", R"(
            annotate_d([[1, 2, 3, 4]], "array_0_expanded/1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 2, 6), list("rows", 0, 1))))
        )");
    }
    else
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_0", R"(
            expand_dims(
                annotate_d([-1, 0], "array_0",
                list("tile", list("columns", 0, 2)))
            , 0)
        )", R"(
            annotate_d([[-1, 0]], "array_0_expanded/1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 2), list("rows", 0, 1))))
        )");
    }
}

void test_expand_dims_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_1", R"(
            expand_dims(
                annotate_d([1, 2, 3, 4], "array_1",
                list("tile", list("rows", 3, 7)))
            , -2)
        )", R"(
            annotate_d([[1, 2, 3, 4]], "array_1_expanded/1",
                    list("tile", list("columns", 3, 7), list("rows", 0, 1)))
        )");
    }
    else
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_1", R"(
            expand_dims(
                annotate_d([-2, -1, 0], "array_1",
                list("tile", list("rows", 0, 3)))
            , -2)
        )", R"(
            annotate_d([[-2, -1, 0]], "array_1_expanded/1",
                    list("tile", list("columns", 0, 3), list("rows", 0, 1)))
        )");
    }
}

void test_expand_dims_1d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_2", R"(
            expand_dims(
                annotate_d([1., 2., 3., 4.], "array_2",
                list("tile", list("columns", 0, 4)))
            , 1)
        )", R"(
            annotate_d([[1.], [2.], [3.], [4.]], "array_2_expanded/1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 1), list("rows", 0, 4))))
        )");
    }
    else
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_2", R"(
            expand_dims(
                annotate_d([5., 6.], "array_2",
                list("tile", list("columns", 4, 6)))
            , 1)
        )", R"(
            annotate_d([[5.], [6.]], "array_2_expanded/1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 1), list("rows", 4, 6))))
        )");
    }
}

void test_expand_dims_1d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_3", R"(
            expand_dims(
                annotate_d([1., 2., 3., 4.], "array_3",
                list("tile", list("columns", 4, 8)))
            , -1)
        )", R"(
            annotate_d([[1.], [2.], [3.], [4.]], "array_3_expanded/1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 1), list("rows", 4, 8))))
        )");
    }
    else
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_3", R"(
            expand_dims(
                annotate_d([-3, -2, -1, 0.], "array_3",
                list("tile", list("columns", 0, 4)))
            , -1)
        )", R"(
            annotate_d([[-3.], [-2.], [-1.], [0.]], "array_3_expanded/1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 1), list("rows", 0, 4))))
        )");
    }
}

void test_expand_dims_1d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_4", R"(
            expand_dims([1., 2., 3., 4.], 1)
        )", R"(
            [[1.], [2.], [3.], [4.]]
        )");
    }
    else
    {
        test_expand_dims_operation("test_expand_dims_2loc1d_4", R"(
            expand_dims([5., 6.], 1)
        )", R"(
            [[5.], [6.]]
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_expand_dims_1d_0();
    test_expand_dims_1d_1();
    test_expand_dims_1d_2();
    test_expand_dims_1d_3();
    test_expand_dims_1d_4();

    hpx::finalize();
    return hpx::util::report_errors();
}
int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    return hpx::init(argc, argv, cfg);
}

