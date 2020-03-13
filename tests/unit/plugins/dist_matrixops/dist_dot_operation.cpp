//   Copyright (c) 2017-2019 Hartmut Kaiser
//   Copyright (c) 2019 Bita Hasheminezhad
//   Copyright (c) 2019 Maxwell Reeser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
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

///////////////////////////////////////////////////////////////////////////////
void test_dot_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(name, code), compile_and_run(name, expected_str));
}

////////////////////////////////////////////////////////////////////////////////
void test_dot_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d_0", R"(
            dot_d(
                annotate_d([1, 2, 3], "test1d_0_1",
                    list("tile", list("columns", 0, 3))),
                annotate_d([4, 5, 6], "test1d_0_2",
                    list("tile", list("columns", 3, 6)))
            )
        )", "91");
    }
    else
    {
        test_dot_operation("test1d_0", R"(
            dot_d(
                annotate_d([4, 5, 6], "test1d_0_1",
                    list("tile", list("columns", 3, 6))),
                annotate_d([1, 2, 3], "test1d_0_2",
                    list("tile", list("columns", 0, 3)))
            )
        )", "91");
    }
}

void test_dot_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d_1", R"(
            dot_d(
                annotate_d([1, 2, 3], "test1d_1_1",
                    list("tile", list("columns", 0, 3))),
                [1, 2, 3, 4, 5, 6]
            )
        )", "91");
    }
    else
    {
        test_dot_operation("test1d_1", R"(
            dot_d(
                annotate_d([4, 5, 6], "test1d_1_1",
                    list("tile", list("columns", 3, 6))),
                [1, 2, 3, 4, 5, 6]
            )
        )", "91");
    }
}

void test_dot_1d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d_2", R"(
            dot_d(
                [1, 2, 3, 4, 5, 6],
                annotate_d([1, 2, 3], "test1d_2_1",
                    list("tile", list("columns", 0, 3)))
            )
        )", "91");
    }
    else
    {
        test_dot_operation("test1d_2", R"(
            dot_d(
                [1, 2, 3, 4, 5, 6],
                annotate_d([4, 5, 6], "test1d_2_1",
                    list("tile", list("columns", 3, 6)))
            )
        )", "91");
    }
}

void test_dot_1d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d_3", R"(
            dot_d(
                annotate_d([1, 2, 3], "test1d_3_1",
                    list("tile", list("rows", 0, 3))),
                annotate_d([0, 0, 0], "test1d_3_2",
                    list("tile", list("columns", 3, 6)))
            )
        )", "14");
    }
    else
    {
        test_dot_operation("test1d_0", R"(
            dot_d(
                annotate_d([4, 5, 6], "test1d_3_1",
                    list("tile", list("rows", 3, 6))),
                annotate_d([1, 2, 3], "test1d_3_2",
                    list("tile", list("columns", 0, 3)))
            )
        )", "14");
    }
}

////////////////////////////////////////////////////////////////////////////////
void test_dot_1d2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d2d_0", R"(
            dot_d(
                annotate_d([1, 2, 3], "test1d2d_0_1",
                    list("tile", list("rows", 0, 3))),
                annotate_d([[4, 4], [5, 5], [6, 0]], "test1d2d_0_2",
                    list("tile", list("columns", 0, 2), list("rows", 3, 6)))
            )
        )", "[91, 55]");
    }
    else
    {
        test_dot_operation("test1d2d_0", R"(
            dot_d(
                annotate_d([4, 5, 6], "test1d2d_0_1",
                    list("tile", list("rows", 3, 6))),
                annotate_d([[1, 1], [2, 2], [3, 3]], "test1d2d_0_2",
                    list("tile", list("columns", 0, 2), list("rows", 0, 3)))
            )
        )", "[91, 55]");
    }
}

void test_dot_1d2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d2d_1", R"(
            dot_d(
                [1, 2, 3, 4, 5, 6],
                annotate_d([[1, 1], [2, 2], [3, 3], [4, 4]], "test1d2d_1_2",
                    list("tile", list("columns", 0, 2), list("rows", 0, 4)))
            )
        )",
            "[91, 55]");
    }
    else
    {
        test_dot_operation("test1d2d_1", R"(
            dot_d(
                [1, 2, 3, 4, 5, 6],
                annotate_d([[5, 5], [6, 0]], "test1d2d_1_2",
                    list("tile", list("columns", 0, 2), list("rows", 4, 6)))
            )
        )", "[91, 55]");
    }
}

void test_dot_1d2d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d2d_2", R"(
            dot_d(
                annotate_d([1, 2, 3], "test1d2d_2_1",
                    list("tile", list("columns", 0, 3))),
                [[1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 0]]
            )
        )", "[91, 55]");
    }
    else
    {
        test_dot_operation("test2d1d_2", R"(
            dot_d(
                annotate_d([4, 5, 6], "test1d2d_2_1",
                    list("tile", list("columns", 3, 6))),
                [[1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 0]]
            )
        )", "[91, 55]");
    }
}

void test_dot_1d2d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d2d_3", R"(
            dot_d(
                annotate_d([1, 2, 3], "test1d2d_3_1",
                    list("tile", list("columns", 0, 3))),
                annotate_d([[4, 4], [5, 5], [6, 0]], "test1d2d_3_2",
                    list("tile", list("columns", 0, 2), list("rows", 3, 6)))
            )
        )", "[91, 55]");
    }
    else
    {
        test_dot_operation("test1d2d_3", R"(
            dot_d(
                annotate_d([4, 5, 6], "test1d2d_3_1",
                    list("tile", list("columns", 3, 6))),
                annotate_d([[1, 1], [2, 2], [3, 3]], "test1d2d_3_2",
                    list("tile", list("columns", 0, 2), list("rows", 0, 3)))
            )
        )", "[91, 55]");
    }
}

void test_dot_1d2d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d2d_4", R"(
            dot_d(
                annotate_d([1, 2, 3], "test1d2d_4_1",
                    list("tile", list("columns", 0, 3))),
                annotate_d([[1], [2], [3], [4], [5], [6]], "test1d2d_4_2",
                    list("tile", list("columns", 0, 1), list("rows", 0, 6)))
            )
        )",
            "[91, 55, 30]");
    }
    else
    {
        test_dot_operation("test1d2d_4", R"(
            dot_d(
                annotate_d([4, 5, 6], "test1d2d_4_1",
                    list("tile", list("columns", 3, 6))),
                annotate_d([[1, 1], [2, 2], [3, 3], [4, 4], [5, 0], [0, 0]],
                    "test1d2d_4_2",
                    list("tile", list("columns", 1, 3), list("rows", 0, 6)))
            )
        )", "[91, 55, 30]");
    }
}

////////////////////////////////////////////////////////////////////////////////
void test_dot_2d1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d1d_0", R"(
            dot_d(
                annotate_d([[1, 2, 3], [1, 2, 3]], "test2d1d_0_1",
                    list("tile", list("columns", 0, 3), list("rows", 0, 2))),
                annotate_d([4, 5, 6], "test2d1d_0_2",
                    list("tile", list("columns", 3, 6)))
            )
        )", "[91, 91]");
    }
    else
    {
        test_dot_operation("test2d1d_0", R"(
            dot_d(
                annotate_d([[4, 5, 6], [4, 5, 6]], "test2d1d_0_1",
                    list("tile", list("columns", 3, 6), list("rows", 0, 2))),
                annotate_d([1, 2, 3], "test2d1d_0_2",
                    list("tile", list("columns", 0, 3)))
            )
        )", "[91, 91]");
    }
}

void test_dot_2d1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d1d_1", R"(
            dot_d(
                annotate_d([[1, 2, 3], [1, 2, 3]], "test2d1d_1_1",
                    list("tile", list("columns", 0, 3), list("rows", 0, 2))),
                [1, 2, 3, 4, 5, 6]
            )
        )", "[91, 91]");
    }
    else
    {
        test_dot_operation("test2d1d_1", R"(
            dot_d(
                annotate_d([[4, 5, 6], [4, 5, 6]], "test2d1d_1_1",
                    list("tile", list("columns", 3, 6), list("rows", 0, 2))),
                [1, 2, 3, 4, 5, 6]
            )
        )", "[91, 91]");
    }
}

void test_dot_2d1d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d1d_2", R"(
            dot_d(
                [[1, 2, 3, 4, 5, 6], [1, 2, 3, 4, 5, 6]],
                annotate_d([4, 5, 6], "test2d1d_2_2",
                    list("tile", list("columns", 3, 6)))
            )
        )", "[91, 91]");
    }
    else
    {
        test_dot_operation("test2d1d_2", R"(
            dot_d(
                [[1, 2, 3, 4, 5, 6], [1, 2, 3, 4, 5, 6]],
                annotate_d([1, 2, 3], "test2d1d_2_2",
                    list("tile", list("columns", 0, 3)))
            )
        )", "[91, 91]");
    }
}

void test_dot_2d1d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d1d_3", R"(
            dot_d(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d1d_3_1",
                    list("tile", list("columns", 0, 6), list("rows", 0, 1))),
                annotate_d([4, 5, 6], "test2d1d_3_2",
                    list("tile", list("columns", 3, 6)))
            )
        )", R"(
            annotate_d([91], "test2d1d_3_1/1",
                list("tile", list("columns", 0, 1)))
        )");
    }
    else
    {
        test_dot_operation("test2d1d_3", R"(
            dot_d(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d1d_3_1",
                    list("tile", list("columns", 0, 6), list("rows", 1, 2))),
                annotate_d([1, 2, 3], "test2d1d_3_2",
                    list("tile", list("columns", 0, 3)))
            )
        )", R"(
            annotate_d([91], "test2d1d_3_1/1",
                list("tile", list("columns", 1, 2)))
        )");
    }
}

void test_dot_2d1d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d1d_4", R"(
            dot_d(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d1d_4_1",
                    list("tile", list("columns", 0, 6), list("rows", 0, 1))),
                [1, 2, 3, 4, 5, 6]
            )
        )", R"(
            annotate_d([91], "test2d1d_4_1/1",
                list("tile", list("columns", 0, 1)))
        )");
    }
    else
    {
        test_dot_operation("test2d1d_4", R"(
            dot_d(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d1d_4_1",
                    list("tile", list("columns", 0, 6), list("rows", 1, 2))),
                [1, 2, 3, 4, 5, 6]
            )
        )", R"(
            annotate_d([91], "test2d1d_4_1/1",
                list("tile", list("columns", 1, 2)))
        )");
    }
}

////////////////////////////////////////////////////////////////////////////////
void test_dot_2d2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d2d_1", R"(
            dot_d(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d2d_1_1",
                    list("tile", list("columns", 0, 6), list("rows", 0, 1))),
                annotate_d([[4, 4], [5, 5], [6, 6]], "test2d2d_1_2",
                    list("tile", list("columns", 0, 2), list("rows", 3, 6)))
            )
        )",
            R"(
            annotate_d([[91, 91]], "test2d2d_1_1/1",
                list("tile", list("rows", 0, 1), list("columns", 0, 2)))
        )");
    }
    else
    {
        test_dot_operation("test2d2d_1", R"(
            dot_d(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d2d_1_1",
                    list("tile", list("columns", 0, 6), list("rows", 1, 2))),
                annotate_d([[1, 1], [2, 2], [3, 3]], "test2d2d_1_2",
                    list("tile", list("columns", 0, 2), list("rows", 0, 3)))
            )
        )",
            R"(
            annotate_d([[91, 91]], "test2d2d_1_1/1",
                list("tile", list("columns", 0, 2), list("rows", 1, 2)))
        )");
    }
}

void test_dot_2d2d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d2d_2", R"(
            dot_d(
                annotate_d([[1, 2, 3], [1, 2, 3]], "test2d2d_2_1",
                    list("tile", list("columns", 0, 3), list("rows", 0, 2))),
                [[1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6]]
            )
        )","[[91, 91], [91, 91]]"
        );
    }
    else
    {
        test_dot_operation("test2d2d_2", R"(
            dot_d(
                annotate_d([[4, 5, 6], [4, 5, 6]], "test2d2d_2_1",
                    list("tile", list("columns", 3, 6), list("rows", 0, 2))),
                [[1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6]]
            )
        )", "[[91, 91], [91, 91]]");
    }
}

void test_dot_2d2d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d2d_3", R"(
            dot_d(
                [[1, 2, 3, 4, 5, 6], [1, 2, 3, 4, 5, 6]],
                annotate_d([[4, 4], [5, 5], [6, 6]], "test2d2d_3_2",
                    list("tile", list("columns", 0, 2), list("rows", 3, 6)))
            )
        )",
            "[[91, 91], [91, 91]]");
    }
    else
    {
        test_dot_operation("test2d2d_3", R"(
            dot_d(
                [[1, 2, 3, 4, 5, 6], [1, 2, 3, 4, 5, 6]],
                annotate_d([[1, 1], [2, 2], [3, 3]], "test2d2d_3_2",
                    list("tile", list("columns", 0, 2), list("rows", 0, 3)))
            )
        )",
            "[[91, 91], [91, 91]]");
    }
}

void test_dot_2d2d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d2d_4", R"(
            dot_d(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d2d_4_1",
                    list("tile", list("columns", 0, 6), list("rows", 0, 1))),
                [[1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6]]
            )
        )",
            R"(
            annotate_d([[91, 91]], "test2d2d_4_1/1",
                list("tile", list("columns", 0, 2), list("rows", 0, 1)))
        )");
    }
    else
    {
        test_dot_operation("test2d2d_4", R"(
            dot_d(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d2d_4_1",
                    list("tile", list("columns", 0, 6), list("rows", 1, 2))),
                [[1, 1], [2, 2], [3, 3], [4, 4], [5, 5], [6, 6]]
            )
        )",
            R"(
            annotate_d([[91, 91]], "test2d2d_4_1/1",
                list("tile", list("columns", 0, 2), list("rows", 1, 2)))
        )");
    }
}

void test_dot_2d2d_5()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d2d_5", R"(
            dot_d(
                annotate_d([[0, 2, 3, 4, 5, 6]], "test2d2d_5_1",
                    list("tile", list("columns", 0, 6), list("rows", 0, 1))),
                annotate_d([[1], [2], [3], [4], [5], [6]], "test2d2d_5_2",
                    list("tile", list("columns", 0, 1), list("rows", 0, 6)))
            )
        )",
            R"(
            annotate_d([[90, 54, 29]], "test2d2d_5_1/1",
                list("tile", list("columns", 0, 3), list("rows", 0, 1)))
        )");
    }
    else
    {
        test_dot_operation("test2d2d_5", R"(
            dot_d(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d2d_5_1",
                    list("tile", list("columns", 0, 6), list("rows", 1, 2))),
                annotate_d([[1, 1], [2, 2], [3, 3], [4, 4], [5, 0], [0, 0]],
                    "test2d2d_5_2",
                    list("tile", list("columns", 1, 3), list("rows", 0, 6)))
            )
        )",
            R"(
            annotate_d([[91, 55, 30]], "test2d2d_5_1/1",
                list("tile", list("columns", 0, 3), list("rows", 1, 2)))
        )");
    }
}

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_dot_1d_0();
    test_dot_1d_1();
    test_dot_1d_2();
    test_dot_1d_3();

    test_dot_1d2d_0();
    test_dot_1d2d_1();
    test_dot_1d2d_2();
    test_dot_1d2d_3();
    test_dot_1d2d_4();

    test_dot_2d1d_0();
    test_dot_2d1d_1();
    test_dot_2d1d_2();
    test_dot_2d1d_3();
    test_dot_2d1d_4();

    test_dot_2d2d_1();
    test_dot_2d2d_2();
    test_dot_2d2d_3();
    test_dot_2d2d_4();
    test_dot_2d2d_5();

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
