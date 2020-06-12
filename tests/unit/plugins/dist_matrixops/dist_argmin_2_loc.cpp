// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

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

void test_argmin_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_argmin_d_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_0", R"(
            argmin_d(annotate_d([42.0, 13.0], "array_0",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2)))))
        )", "2");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_0", R"(
            argmin_d(annotate_d([1.0, 2.0, 33.0], "array_0",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 2, 5)))))
        )", "2");
    }
}

void test_argmin_d_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_1", R"(
            argmin_d(annotate_d([42.0, -13.0, 1.0], "array_0",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("rows", 0, 3)))))
        )", "1");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_1", R"(
            argmin_d(annotate_d([1.0, 2.0, 33.0], "array_0",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("rows", 3, 6)))))
        )", "1");
    }
}

void test_argmin_d_1d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_2", R"(
            argmin_d(annotate_d([42.0, 13.0], "array_2",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2)))))
        )", "1");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_2", R"(
            argmin_d(annotate_d([13.0, 22.0, 33.0], "array_2",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 2, 5)))))
        )", "1");
    }
}

void test_argmin_d_1d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_3", R"(
            argmin_d(annotate_d([13.0, 22.0, 33.0], "array_3",
                list("tile", list("columns", 2, 5))))
        )", "1");

    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_3", R"(
            argmin_d(annotate_d([42.0, 13.0], "array_3",
                list("tile", list("columns", 0, 2))))
        )", "1");
    }
}

void test_argmin_d_1d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_4", R"(
            argmin_d(annotate_d(astype([], "int"), "array_4",
                list("tile", list("rows", 0, 0))))
        )", "2");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_4", R"(
            argmin_d(annotate_d([42, 13, -13], "array_4",
                list("tile", list("rows", 0, 3))))
        )", "2");
    }
}

void test_argmin_d_1d_5()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_5", R"(
            argmin_d(annotate_d([42, 13, -13], "array_5",
                list("tile", list("rows", 0, 3))))
        )", "2");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc1d_5", R"(
            argmin_d(annotate_d(astype([], "int"), "array_5",
                list("tile", list("rows", 0, 0))))
        )", "2");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_argmin_d_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_0", R"(
            argmin_d(annotate_d([[4, 1, 2, 4], [1, 2, 3, 4]], "array2d_0",
                list("tile", list("columns", 0, 4), list("rows", 0, 2))))
        )", "8");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_0", R"(
            argmin_d(annotate_d([[0, 3, 2, 0]], "array2d_0",
                list("tile", list("columns", 0, 4), list("rows", 2, 3))))
        )", "8");
    }
}

void test_argmin_d_2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_1", R"(
            argmin_d(annotate_d([[4, 1, 2, 4], [1, 2, 3, 4]], "array2d_1",
                list("tile", list("columns", 0, 4), list("rows", 0, 2))), -2)
        )", "[2, 0, 0, 2]");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_1", R"(
            argmin_d(annotate_d([[0, 3, 2, 0]], "array2d_1",
                list("tile", list("columns", 0, 4), list("rows", 2, 3))), -2)
        )", "[2, 0, 0, 2]");
    }
}

void test_argmin_d_2d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_2", R"(
            argmin_d(annotate_d([[4, 1, 2, 4], [1, 2, 3, 4]], "array2d_2",
                list("tile", list("columns", 0, 4), list("rows", 0, 2))), 1)
        )", R"(
            annotate_d([1, 0], "array2d_2/1",
                list("tile", list("columns", 0, 2)))
        )");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_2", R"(
            argmin_d(annotate_d([[0, 3, 2, 0]], "array2d_2",
                list("tile", list("columns", 0, 4), list("rows", 2, 3))), 1)
        )", R"(
            annotate_d([0], "array2d_2/1", list("tile", list("columns", 2, 3)))
        )");
    }
}

void test_argmin_d_2d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_3", R"(
            argmin_d(annotate_d([[2, 3, 4], [0, 0, 5], [5, 2, 3]], "array2d_3",
                list("tile", list("columns", 1, 4), list("rows", 0, 3))))
        )", "5");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_3", R"(
            argmin_d(annotate_d([[1], [1], [0]], "array2d_3",
                list("tile", list("columns", 0, 1), list("rows", 0, 3))))
        )", "5");
    }
}

void test_argmin_d_2d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_4", R"(
            argmin_d(annotate_d([[2, 3, 4], [0, 0, 5], [5, 2, 3]], "array2d_4",
                list("tile", list("columns", 1, 4), list("rows", 0, 3))), 0)
        )", R"(
            annotate_d([1, 1, 2], "array2d_4/1",
                list("tile", list("columns", 1, 4)))
        )");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_4", R"(
            argmin_d(annotate_d([[1], [1], [0]], "array2d_4",
                list("tile", list("columns", 0, 1), list("rows", 0, 3))), 0)
        )", R"(
            annotate_d([2], "array2d_4/1", list("tile", list("columns", 0, 1)))
        )");
    }
}

void test_argmin_d_2d_5()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_5", R"(
            argmin_d(annotate_d([[2, 3, 4], [0, 0, 5], [5, 2, 3]], "array2d_5",
                list("tile", list("columns", 1, 4), list("rows", 0, 3))), -1)
        )", "[0, 1, 0]");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_5", R"(
            argmin_d(annotate_d([[1], [1], [0]], "array2d_5",
                list("tile", list("columns", 0, 1), list("rows", 0, 3))), -1)
        )", "[0, 1, 0]");
    }
}

void test_argmin_d_2d_6()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_6", R"(
            argmin_d(annotate_d([[2, 3, 4], [0, 0, 5], [5, 2, 3]], "array2d_6",
                list("tile", list("columns", 0, 3), list("rows", 0, 3))), 0)
        )", R"(
            annotate_d([1, 1, 2], "array2d_6/1",
                list("tile", list("columns", 0, 3)))
        )");
    }
    else
    {
        test_argmin_d_operation("test_argmin_d_2loc2d_6", R"(
            argmin_d(annotate_d(astype([[]], "int"), "array2d_6",
                list("tile", list("columns", 0, 0), list("rows", 0, 0))), 0)
        )", R"(
            annotate_d(astype([], "int"), "array2d_6/1",
                list("tile", list("columns", 0, 0)))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_argmin_d_1d_0();
    test_argmin_d_1d_1();
    test_argmin_d_1d_2();
    test_argmin_d_1d_3();
    test_argmin_d_1d_4();
    test_argmin_d_1d_5();

    test_argmin_d_2d_0();
    test_argmin_d_2d_1();
    test_argmin_d_2d_2();
    test_argmin_d_2d_3();
    test_argmin_d_2d_4();
    test_argmin_d_2d_5();
    test_argmin_d_2d_6();

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

