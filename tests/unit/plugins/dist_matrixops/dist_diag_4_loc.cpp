//   Copyright (c) 2020 Hartmut Kaiser
//   Copyright (c) 2020 Nanmiao Wu
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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

///////////////////////////////////////////////////////////////////////////////
void test_diag_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

////////////////////////////////////////////////////////////////////////////////
void test_diag_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([1, 2], "my_diag1d_0",
                    list("tile", list("columns", 0, 2))
                ),
                2, "row", 0, 4
            )
        )", R"(
            annotate_d([[0, 0, 1, 0, 0, 0, 0], [0, 0, 0, 2, 0, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 7),
                    list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([3], "my_diag1d_0",
                    list("tile", list("columns", 2, 3))
                ),
                2, "row", 1, 4
            )
        )", R"(
            annotate_d([[0, 0, 0, 0, 3, 0, 0], [0, 0, 0, 0, 0, 4, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 0, 7),
                    list("rows", 2, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([4], "my_diag1d_0",
                    list("tile", list("columns", 3, 4))
                ),
                2, "row", 2, 4
            )
        )", R"(
            annotate_d([[0, 0, 0, 0, 0, 0, 5], [0, 0, 0, 0, 0, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 7),
                    list("rows", 4, 6))))
        )");
    }
    else
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([5], "my_diag1d_0",
                    list("tile", list("columns", 4, 5))
                ),
                2, "row", 3, 4
            )
        )", R"(
            annotate_d([[0, 0, 0, 0, 0, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 0, 7),
                    list("rows", 6, 7))))
        )");
    }
}

void test_diag_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_diag_d_operation("test1d_1", R"(
            diag_d(
                annotate_d([1], "my_diag1d_1",
                    list("tile", list("columns", 0, 1))
                ),
                -2, "row", 0, 4
            )
        )", R"(
            annotate_d([[0, 0, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0]],
                "my_diag1d_1_diag/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 8),
                    list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_diag_d_operation("test1d_1", R"(
            diag_d(
                annotate_d([2, 3], "my_diag1d_1",
                    list("tile", list("columns", 1, 3))
                ),
                -2, "row", 1, 4
            )
        )", R"(
            annotate_d([[1, 0, 0, 0, 0, 0, 0, 0],
                [0, 2, 0, 0, 0, 0, 0, 0]],
                "my_diag1d_1_diag/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 0, 8),
                    list("rows", 2, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_diag_d_operation("test1d_1", R"(
            diag_d(
                annotate_d([4], "my_diag1d_1",
                    list("tile", list("columns", 3, 4))
                ),
                -2, "row", 2, 4
            )
        )", R"(
            annotate_d([[0, 0, 3, 0, 0, 0, 0, 0],
                [0, 0, 0, 4, 0, 0, 0, 0]],
                "my_diag1d_1_diag/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 8),
                    list("rows", 4, 6))))
        )");
    }
    else
    {
        test_diag_d_operation("test1d_1", R"(
            diag_d(
                annotate_d([5, 6], "my_diag1d_1",
                    list("tile", list("columns", 4, 6))
                ),
                -2, "row", 3, 4
            )
        )", R"(
            annotate_d([[0, 0, 0, 0, 5, 0, 0, 0],
                [0, 0, 0, 0, 0, 6, 0, 0]],
                "my_diag1d_1_diag/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 0, 8),
                    list("rows", 6, 8))))
        )");
    }
}

void test_diag_1d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_diag_d_operation("test1d_2", R"(
            diag_d(
                annotate_d([1, 2], "my_diag1d_2",
                    list("tile", list("columns", 0, 2))
                ),
                2, "column", 0, 4
            )
        )", R"(
            annotate_d([[0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]],
                "my_diag1d_2_diag/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 2),
                    list("rows", 0, 7))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_diag_d_operation("test1d_2", R"(
            diag_d(
                annotate_d([3], "my_diag1d_2",
                    list("tile", list("columns", 2, 3))
                ),
                2, "column", 1, 4
            )
        )", R"(
            annotate_d([[1, 0], [0, 2], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]],
                "my_diag1d_2_diag/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 2, 4),
                    list("rows", 0, 7))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_diag_d_operation("test1d_2", R"(
            diag_d(
                annotate_d([4], "my_diag1d_2",
                    list("tile", list("columns", 3, 4))
                ),
                2, "column", 2, 4
            )
        )", R"(
            annotate_d([[0, 0], [0, 0], [3, 0], [0, 4], [0, 0], [0, 0], [0, 0]],
                "my_diag1d_2_diag/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 4, 6),
                    list("rows", 0, 7))))
        )");
    }
    else
    {
        test_diag_d_operation("test1d_2", R"(
            diag_d(
                annotate_d([5], "my_diag1d_2",
                    list("tile", list("columns", 4, 5))
                ),
                2, "column", 3, 4
            )
        )", R"(
            annotate_d([[0], [0], [0], [0], [5], [0], [0]],
                "my_diag1d_2_diag/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 6, 7),
                    list("rows", 0, 7))))
        )");
    }
}

void test_diag_1d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_diag_d_operation("test1d_3", R"(
            diag_d(
                annotate_d([1], "my_diag1d_3",
                    list("tile", list("columns", 0, 1))
                ),
                -2, "column", 0, 4
            )
        )", R"(
            annotate_d([[0, 0], [0, 0], [1, 0], [0, 2],
                [0, 0], [0, 0], [0, 0], [0, 0]],
                "my_diag1d_3_diag/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 2),
                    list("rows", 0, 8))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_diag_d_operation("test1d_3", R"(
            diag_d(
                annotate_d([2, 3], "my_diag1d_3",
                    list("tile", list("columns", 1, 3))
                ),
                -2, "column", 1, 4
            )
        )", R"(
            annotate_d([[0, 0], [0, 0], [0, 0], [0, 0],
                [3, 0], [0, 4], [0, 0], [0, 0]],
                "my_diag1d_3_diag/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 2, 4),
                    list("rows", 0, 8))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_diag_d_operation("test1d_3", R"(
            diag_d(
                annotate_d([4], "my_diag1d_3",
                    list("tile", list("columns", 3, 4))
                ),
                -2, "column", 2, 4
            )
        )", R"(
            annotate_d([[0, 0], [0, 0], [0, 0], [0, 0],
                [0, 0], [0, 0], [5, 0], [0, 6]],
                "my_diag1d_3_diag/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 4, 6),
                    list("rows", 0, 8))))
        )");
    }
    else
    {
        test_diag_d_operation("test1d_3", R"(
            diag_d(
                annotate_d([5, 6], "my_diag1d_3",
                    list("tile", list("columns", 4, 6))
                ),
                -2, "column", 3, 4
            )
        )", R"(
            annotate_d([[0, 0], [0, 0], [0, 0], [0, 0],
                [0, 0], [0, 0], [0, 0], [0, 0]],
                "my_diag1d_3_diag/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 6, 8),
                    list("rows", 0, 8))))
        )");
    }
}

void test_diag_1d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_diag_d_operation("test1d_4", R"(
            diag_d(
                annotate_d([1], "my_diag1d_4",
                    list("tile", list("columns", 0, 1))
                ),
                -2, "sym", 0, 4
            )
        )", R"(
            annotate_d([[0, 0, 0, 0], [0, 0, 0, 0],
                [1, 0, 0, 0], [0, 2, 0, 0]],
                "my_diag1d_4_diag/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 4),
                    list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_diag_d_operation("test1d_4", R"(
            diag_d(
                annotate_d([2, 3], "my_diag1d_4",
                    list("tile", list("columns", 1, 3))
                ),
                -2, "sym", 1, 4
            )
        )", R"(
            annotate_d([[0, 0, 0, 0], [0, 0, 0, 0],
                [0, 0, 0, 0], [0, 0, 0, 0]],
                "my_diag1d_4_diag/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 4, 8),
                    list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_diag_d_operation("test1d_4", R"(
            diag_d(
                annotate_d([4], "my_diag1d_4",
                    list("tile", list("columns", 3, 4))
                ),
                -2, "sym", 2, 4
            )
        )", R"(
            annotate_d([[0, 0, 3, 0], [0, 0, 0, 4],
                [0, 0, 0, 0], [0, 0, 0, 0]],
                "my_diag1d_4_diag/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 4),
                    list("rows", 4, 8))))
        )");
    }
    else
    {
        test_diag_d_operation("test1d_4", R"(
            diag_d(
                annotate_d([5, 6], "my_diag1d_4",
                    list("tile", list("columns", 4, 6))
                ),
                -2, "sym", 3, 4
            )
        )", R"(
            annotate_d([[0, 0, 0, 0], [0, 0, 0, 0],
                [5, 0, 0, 0], [0, 6, 0, 0]],
                "my_diag1d_4_diag/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 4, 8),
                    list("rows", 4, 8))))
        )");
    }
}

void test_diag_1d_5()
{
    if (hpx::get_locality_id() == 0)
    {
        test_diag_d_operation("test1d_5", R"(
            diag_d(
                annotate_d([1, 2], "my_diag1d_5",
                    list("tile", list("columns", 0, 2))
                ),
                2
            )
        )", R"(
            annotate_d([[0, 0, 1, 0], [0, 0, 0, 2],
                [0, 0, 0, 0], [0, 0, 0, 0]],
                "my_diag1d_5_diag/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 4),
                    list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_diag_d_operation("test1d_5", R"(
            diag_d(
                annotate_d([3], "my_diag1d_5",
                    list("tile", list("columns", 2, 3))
                ),
                2
            )
        )", R"(
            annotate_d([[0, 0, 0], [0, 0, 0],
                [3, 0, 0], [0, 4, 0]],
                "my_diag1d_5_diag/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 4, 7),
                    list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_diag_d_operation("test1d_5", R"(
            diag_d(
                annotate_d([4], "my_diag1d_5",
                    list("tile", list("columns", 3, 4))
                ),
                2
            )
        )", R"(
            annotate_d([[0, 0, 0, 0], [0, 0, 0, 0],
                [0, 0, 0, 0]],
                "my_diag1d_5_diag/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 4),
                    list("rows", 4, 7))))
        )");
    }
    else
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([5], "my_diag1d_5",
                    list("tile", list("columns", 4, 5))
                ),
                2
            )
        )", R"(
            annotate_d([[0, 0, 5], [0, 0, 0],
                [0, 0, 0]],
                "my_diag1d_5_diag/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 4, 7),
                    list("rows", 4, 7))))
        )");
    }
}
////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_diag_1d_0();
    test_diag_1d_1();
    test_diag_1d_2();
    test_diag_1d_3();
    test_diag_1d_4();
    test_diag_1d_5();

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
