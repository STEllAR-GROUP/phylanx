// Copyright (c) 2020 Bita Hashemaxezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
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

void test_argmax_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_argmax_d_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_0", R"(
            argmax_d(annotate_d([42.0, 13.0, 2.0], "array_0",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 3)))))
        )", "0");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_0", R"(
            argmax_d(annotate_d([1.0, 2.0, 33.0], "array_0",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 3, 6)))))
        )", "0");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_0", R"(
            argmax_d(annotate_d([4.0, 12.0, 14.0], "array_0",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 6, 9)))))
        )", "0");
    }
}

void test_argmax_d_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_1", R"(
            argmax_d(annotate_d([42., 13., 43.], "array_1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 3)))))
        )", "2");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_1", R"(
            argmax_d(annotate_d([], "array_1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 0, 0)))))
        )", "2");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_1", R"(
            argmax_d(annotate_d([], "array_1",
                list("tile", list("columns", 0, 0))))
        )", "2");
    }
}

void test_argmax_d_1d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_2", R"(
            argmax_d(annotate_d([42.0, 13.0, 2.0], "array_2",
                list("tile", list("columns", 0, 3))))
        )", "0");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_2", R"(
            argmax_d(annotate_d([1.0, 42.0, 33.0], "array_2",
                list("tile", list("columns", 3, 6))))
        )", "0");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_2", R"(
            argmax_d(annotate_d([42.0, 12.0, 14.0], "array_2",
                list("tile", list("columns", 6, 9))))
        )", "0");
    }
}

void test_argmax_d_1d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_3", R"(
            argmax_d(annotate_d([42.0, 13.0, 2.0], "array_3",
                list("tile", list("columns", 3, 6))))
        )", "1");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_3", R"(
            argmax_d(annotate_d([1.0, 42.0, 33.0], "array_3",
                list("tile", list("columns", 0, 3))))
        )", "1");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc1d_3", R"(
            argmax_d(annotate_d([42.0, 12.0], "array_3",
                list("tile", list("columns", 6, 8))))
        )", "1");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_argmax_d_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_0", R"(
            argmax_d(annotate_d([[4.0], [1.0], [4.0]], "array_0",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 1), list("rows", 0, 3)))))
        )", "10");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_0", R"(
            argmax_d(annotate_d([[1.0, 2.0, 4.0], [2.0, 3.0, 4.0]], "array_0",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 1, 4), list("rows", 0, 2)))))
        )", "10");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_0", R"(
            argmax_d(annotate_d([[3.0, 5.0, 0.0]], "array_0",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 1, 4), list("rows", 2, 3)))))
        )", "10");
    }
}

void test_argmax_d_2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_1", R"(
            argmax_d(annotate_d([[2, 4], [3, 4], [4, 0]], "array_1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 2, 4), list("rows", 0, 3)))))
        )", "3");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_1", R"(
            argmax_d(annotate_d([[4, 3]], "array_1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 0, 2), list("rows", 2, 3)))))
        )", "3");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_1", R"(
            argmax_d(annotate_d([[0, 1], [1, 2]], "array_1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 0, 2), list("rows", 0, 2)))))
        )", "3");
    }
}


void test_argmax_d_2d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_2", R"(
            argmax_d(annotate_d([[1, 2, 3, 4]], "array_2",
                list("tile", list("columns", 0, 4), list("rows", 1, 2))))
        )", "3");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_2", R"(
            argmax_d(annotate_d([[0, 1, 2, 4]], "array_2",
                list("tile", list("columns", 0, 4), list("rows", 0, 1))))
        )", "3");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_2", R"(
            argmax_d(annotate_d([[4, 3, 4, 0]], "array_2",
                list("tile", list("columns", 0, 4), list("rows", 2, 3))))
        )", "3");
    }
}

void test_argmax_d_2d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_3", R"(
            argmax_d(annotate_d([[2, 4], [3, 4], [4, 0]], "array_3",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 2, 4), list("rows", 0, 3)))),
            0)
        )", R"(
            annotate_d([2, 0], "array_3/1", list("tile", list("columns", 2, 4)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_3", R"(
            argmax_d(annotate_d([[1], [2], [3]], "array_3",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 1, 2), list("rows", 0, 3)))),
            0)
        )", R"(
            annotate_d([2], "array_3/1", list("tile", list("columns", 1, 2)))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_3", R"(
            argmax_d(annotate_d([[0], [1], [4]], "array_3",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 0, 1), list("rows", 0, 3)))),
            0)
        )", R"(
            annotate_d([2], "array_3/1", list("tile", list("columns", 0, 1)))
        )");
    }
}

void test_argmax_d_2d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_4", R"(
            argmax_d(annotate_d([[2, 4], [3, 4], [4, 0]], "array_4",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 2, 4), list("rows", 0, 3)))),
            1)
        )", "[3, 3, 0]");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_4", R"(
            argmax_d(annotate_d([[1], [2], [3]], "array_4",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 1, 2), list("rows", 0, 3)))),
            1)
        )", "[3, 3, 0]");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_4", R"(
            argmax_d(annotate_d([[0], [1], [4]], "array_4",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 0, 1), list("rows", 0, 3)))),
            1)
        )", "[3, 3, 0]");
    }
}

void test_argmax_d_2d_5()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_5", R"(
            argmax_d(annotate_d([[]], "array_5",
                list("tile", list("columns", 0, 0), list("rows", 0, 0))))
        )", "3");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_5", R"(
            argmax_d(annotate_d([[0, 1, 2, 4], [1, 2, 3, 4], [4, 3, 4, 0]],
                "array_5",
                list("tile", list("columns", 0, 4), list("rows", 0, 3))))
        )", "3");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_5", R"(
            argmax_d(annotate_d([[]], "array_5",
                list("tile", list("columns", 0, 0), list("rows", 0, 0))))
        )", "3");
    }
}

void test_argmax_d_2d_6()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_6", R"(
            argmax_d(annotate_d([[2, 3, 4], [0, 0, 5], [5, 2, 3]], "array2d_6",
                list("tile", list("columns", 0, 3), list("rows", 1, 4))), 0)
        )", "[3, 1, 2]");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_6", R"(
            argmax_d(annotate_d([[]], "array2d_6",
                list("tile", list("columns", 0, 0), list("rows", 0, 0))), 0)
        )", "[3, 1, 2]");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_6", R"(
            argmax_d(annotate_d([[1, 1, 4]], "array2d_6",
                list("tile", list("columns", 0, 3), list("rows", 0, 1))), 0)
        )", "[3, 1, 2]");
    }
}

void test_argmax_d_2d_7()
{
    if (hpx::get_locality_id() == 0)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_7", R"(
            argmax_d(annotate_d([[2, 3, 4], [0, 0, 5], [5, 2, 3]], "array2d_7",
                list("tile", list("columns", 0, 3), list("rows", 0, 3))), -1)
        )", "[2, 2, 3]");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_7", R"(
            argmax_d(annotate_d([[4, 1], [0, 2], [6, 0]], "array2d_7",
                list("tile", list("columns", 3, 5), list("rows", 0, 3))), -1)
        )", "[2, 2, 3]");
    }
    else
    {
        test_argmax_d_operation("test_argmax_d_3loc2d_7", R"(
            argmax_d(annotate_d([[]], "array2d_7",
                list("tile", list("columns", 0, 0), list("rows", 0, 0))), -1)
        )", "[2, 2, 3]");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_argmax_d_1d_0();
    test_argmax_d_1d_1();
    test_argmax_d_1d_2();
    test_argmax_d_1d_3();

    test_argmax_d_2d_0();
    test_argmax_d_2d_1();
    test_argmax_d_2d_2();
    test_argmax_d_2d_3();
    test_argmax_d_2d_4();
    test_argmax_d_2d_5();
    test_argmax_d_2d_6();
    test_argmax_d_2d_7();

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

