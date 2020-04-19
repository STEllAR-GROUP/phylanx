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

void test_retile_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_retile_3loc_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc1d_0", R"(
            retile_d(
                annotate_d([1, 2, 3], "tiled_array_1d_0",
                    list("tile", list("columns", 0, 3))
                ),
                "user", nil, nil, list("tile", list("rows", 0, 2))
            )
        )", R"(
            annotate_d([1, 2], "tiled_array_1d_0_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc1d_0", R"(
            retile_d(
                annotate_d([4, 5, 6], "tiled_array_1d_0",
                    list("tile", list("columns", 3, 6))
                ),
                "user", nil, nil, list("tile", list("rows", 2, 7))
            )
        )", R"(
            annotate_d([3, 4, 5, 6, 7], "tiled_array_1d_0_retiled/1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("rows", 2, 7))))
        )");
    }
    else
    {
        test_retile_d_operation("test_retile_3loc1d_0", R"(
            retile_d(
                annotate_d([7, 8, 9], "tiled_array_1d_0",
                    list("tile", list("columns", 6, 9))
                ),
                "user", nil, nil, list("tile", list("rows", 7, 9))
            )
        )", R"(
            annotate_d([8, 9], "tiled_array_1d_0_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("rows", 7, 9))))
        )");
    }
}

void test_retile_3loc_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc1d_1", R"(
            retile_d(
                annotate_d([1, 2, 3], "tiled_array_1d_1",
                    list("tile", list("columns", 0, 3))
                ),
                "user", nil, nil, list("tile", list("columns", 0, 4))
            )
        )", R"(
            annotate_d([1, 2, 3, 4], "tiled_array_1d_1_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc1d_1", R"(
            retile_d(
                annotate_d([4, 5, 6], "tiled_array_1d_1",
                    list("tile", list("columns", 3, 6))
                ),
                "user", nil, nil, list("tile", list("columns", 4, 7))
            )
        )", R"(
            annotate_d([5, 6, 7], "tiled_array_1d_1_retiled/1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 4, 7))))
        )");
    }
    else
    {
        test_retile_d_operation("test_retile_3loc1d_1", R"(
            retile_d(
                annotate_d([7, 8, 9], "tiled_array_1d_1",
                    list("tile", list("columns", 6, 9))
                ),
                "user", nil, nil, list("tile", list("columns", 7, 9))
            )
        )", R"(
            annotate_d([8, 9], "tiled_array_1d_1_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 7, 9))))
        )");
    }
}

void test_retile_3loc_1d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc1d_2", R"(
            retile_d(
                annotate_d([1, 2, 3], "tiled_array_1d_2",
                    list("tile", list("columns", 0, 3))
                ),
                "user", nil, nil, list("tile", list("columns", 0, 2))
            )
        )", R"(
            annotate_d([1, 2], "tiled_array_1d_2_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc1d_2", R"(
            retile_d(
                annotate_d([4, 5, 6], "tiled_array_1d_2",
                    list("tile", list("columns", 3, 6))
                ),
                "user", nil, nil, list("tile", list("columns", 2, 5))
            )
        )", R"(
            annotate_d([3, 4, 5], "tiled_array_1d_2_retiled/1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 2, 5))))
        )");
    }
    else
    {
        test_retile_d_operation("test_retile_3loc1d_2", R"(
            retile_d(
                annotate_d([7, 8, 9], "tiled_array_1d_2",
                    list("tile", list("columns", 6, 9))
                ),
                "user", nil, nil, list("tile", list("columns", 5, 9))
            )
        )", R"(
            annotate_d([6, 7, 8, 9], "tiled_array_1d_2_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 5, 9))))
        )");
    }
}

void test_retile_3loc_1d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc1d_3", R"(
            retile_d(
                annotate_d([1, 2, 3], "tiled_array_1d_3",
                    list("tile", list("columns", 0, 3))
                ),
                "user", nil, nil, list("tile", list("columns", 3, 7))
            )
        )", R"(
            annotate_d([4, 5, 6, 7], "tiled_array_1d_3_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 3, 7))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc1d_3", R"(
            retile_d(
                annotate_d([4, 5, 6], "tiled_array_1d_3",
                    list("tile", list("columns", 3, 6))
                ),
                "user", nil, nil, list("tile", list("columns", 7, 9))
            )
        )", R"(
            annotate_d([8, 9], "tiled_array_1d_3_retiled/1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 7, 9))))
        )");
    }
    else
    {
        test_retile_d_operation("test_retile_3loc1d_3", R"(
            retile_d(
                annotate_d([7, 8, 9], "tiled_array_1d_3",
                    list("tile", list("columns", 6, 9))
                ),
                "user", nil, nil, list("tile", list("columns", 0, 3))
            )
        )", R"(
            annotate_d([1, 2, 3], "tiled_array_1d_3_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 0, 3))))
        )");
    }
}

void test_retile_3loc_1d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc1d_4", R"(
            retile_d(
                annotate_d([1, 2, 3], "tiled_array_1d_4",
                    list("tile", list("columns", 0, 3))
                ),
                "user", nil, nil, list("tile", list("columns", 4, 7))
            )
        )", R"(
            annotate_d([5, 6, 7], "tiled_array_1d_4_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 4, 7))))
        )");

    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc1d_4", R"(
            retile_d(
                annotate_d([4, 5, 6], "tiled_array_1d_4",
                    list("tile", list("columns", 3, 6))
                ),
                "user", nil, nil, list("tile", list("columns", 0, 4))
            )
        )", R"(
            annotate_d([1, 2, 3, 4], "tiled_array_1d_4_retiled/1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 0, 4))))
        )");
    }
    else
    {
        test_retile_d_operation("test_retile_3loc1d_4", R"(
            retile_d(
                annotate_d([7, 8, 9], "tiled_array_1d_4",
                    list("tile", list("columns", 6, 9))
                ),
                "user", nil, nil, list("tile", list("columns", 7, 9))
            )
        )", R"(
            annotate_d([8, 9], "tiled_array_1d_4_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 7, 9))))
        )");
    }
}

void test_retile_3loc_1d_5()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc1d_5", R"(
            retile_d(
                annotate_d([1, 2, 3], "tiled_array_1d_5",
                    list("tile", list("columns", 0, 3))
                ),
                "user", nil, nil, list("tile", list("columns", 4, 7))
            )
        )", R"(
            annotate_d([5, 6, 7], "tiled_array_1d_5_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 4, 7))))
        )");


    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc1d_5", R"(
            retile_d(
                annotate_d([4, 5, 6], "tiled_array_1d_5",
                    list("tile", list("columns", 3, 6))
                ),
                "user", nil, nil, list("tile", list("columns", 7, 9))
            )
        )", R"(
            annotate_d([8, 9], "tiled_array_1d_5_retiled/1",
                list("tile", list("columns", 7, 9)))
        )");

    }
    else
    {
        test_retile_d_operation("test_retile_3loc1d_5", R"(
            retile_d(
                annotate_d([7, 8, 9], "tiled_array_1d_5",
                    list("tile", list("columns", 6, 9))
                ),
                "user", nil, nil, list("tile", list("columns", 0, 4))
            )
        )", R"(
            annotate_d([1, 2, 3, 4], "tiled_array_1d_5_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 0, 4))))
        )");
    }
}

void test_retile_3loc_1d_6()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc1d_6", R"(
            retile_d(
                annotate_d([1, 2, 3], "tiled_array_1d_6",
                    list("tile", list("columns", 0, 3))
                ),
                "row", list(2), 3
            )
        )", R"(
            annotate_d([1, 2, 3, 4, 5, 6, 7], "tiled_array_1d_6_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("rows", 0, 7))))
        )");


    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc1d_6", R"(
            retile_d(
                annotate_d([4, 5, 6, 7, 8, 9, 10, 11], "tiled_array_1d_6",
                    list("tile", list("columns", 3, 11))
                ),
                "row", list(2), 3
            )
        )", R"(
            annotate_d([5, 6, 7, 8, 9, 10], "tiled_array_1d_6_retiled/1",
                list("tile", list("rows", 4, 10)))
        )");

    }
    else
    {
        test_retile_d_operation("test_retile_3loc1d_6", R"(
            retile_d(
                annotate_d([12, 13], "tiled_array_1d_6",
                    list("tile", list("columns", 11, 13))
                ),
                "row", list(2), 3
            )
        )", R"(
            annotate_d([8, 9, 10, 11, 12, 13], "tiled_array_1d_6_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("rows", 7, 13))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_retile_3loc_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc2d_0", R"(
            retile_d(
                annotate_d([[1, 2, 3], [-1, -2, -3]], "tiled_array_2d_0",
                    list("tile", list("columns", 0, 3), list("rows", 0, 2))
                ),
                "user", nil, nil,
                list("tile", list("columns", 0, 2), list("rows", 0, 4))
            )
        )", R"(
            annotate_d([[1, 2], [-1, -2], [11, 12], [-11, -12]],
                "tiled_array_2d_0_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 2), list("rows", 0, 4))))
        )");


    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc2d_0", R"(
            retile_d(
                annotate_d([[4, 5], [-4, -5], [14, 15], [-14, -15]],
                    "tiled_array_2d_0",
                    list("tile", list("columns", 3, 5), list("rows", 0, 4))
                ),
                "user", nil, nil,
                list("tile", list("columns", 2, 4), list("rows", 0, 4))
            )
        )", R"(
            annotate_d([[3, 4], [-3, -4], [13, 14], [-13, -14]],
                "tiled_array_2d_0_retiled/1",
                list("tile", list("columns", 2, 4), list("rows", 0, 4)))
        )");

    }
    else
    {
        test_retile_d_operation("test_retile_3loc2d_0", R"(
            retile_d(
                annotate_d([[11, 12, 13], [-11, -12, -13]], "tiled_array_2d_0",
                    list("tile", list("columns", 0, 3), list("rows", 2, 4))
                ),
                "user", nil, nil,
                list("tile", list("columns", 4, 5), list("rows", 0, 4))
            )
        )", R"(
            annotate_d([[5], [-5], [15], [-15]], "tiled_array_2d_0_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 4, 5), list("rows", 0, 4))))
        )");
    }
}

void test_retile_3loc_2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc2d_1", R"(
            retile_d(
                annotate_d([[4, 5], [-4, -5], [14, 15], [-14, -15]],
                    "tiled_array_2d_1",
                    list("tile", list("columns", 3, 5), list("rows", 0, 4))
                ),
                "user", nil, nil,
                list("tile", list("columns", 0, 3), list("rows", 0, 4))
            )
        )", R"(
            annotate_d([[1, 2, 3], [-1, -2, -3], [11, 12, 13], [-11, -12, -13]],
                "tiled_array_2d_1_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 3), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc2d_1", R"(
            retile_d(
                annotate_d([[3], [-3], [13], [-13]],
                    "tiled_array_2d_1",
                    list("tile", list("columns", 2, 3), list("rows", 0, 4))
                ),
                "user", nil, nil,
                list("tile", list("columns", 3, 4), list("rows", 0, 4))
            )
        )", R"(
            annotate_d([[4], [-4], [14], [-14]],
                "tiled_array_2d_1_retiled/1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 3, 4), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_retile_d_operation("test_retile_3loc2d_1", R"(
            retile_d(
                annotate_d([[1, 2], [-1, -2], [11, 12], [-11, -12]],
                    "tiled_array_2d_1",
                    list("tile", list("columns", 0, 2), list("rows", 0, 4))
                ),
                "user", nil, nil,
                list("tile", list("columns", 4, 5), list("rows", 0, 4))
            )
        )", R"(
            annotate_d([[5], [-5], [15], [-15]],
                "tiled_array_2d_1_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 4, 5), list("rows", 0, 4))))
        )");
    }
}

void test_retile_3loc_2d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc2d_2", R"(
            retile_d(
                annotate_d([[5, 6], [-5, -6]],
                    "tiled_array_2d_2",
                    list("tile", list("columns", 4, 6), list("rows", 0, 2))
                ),
                "user", nil, nil,
                list("tile", list("columns", 0, 2), list("rows", 0, 2))
            )
        )", R"(
            annotate_d([[1, 2], [-1, -2]], "tiled_array_2d_2_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 2), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc2d_2", R"(
            retile_d(
                annotate_d([[1], [-1]], "tiled_array_2d_2",
                    list("tile", list("columns", 0, 1), list("rows", 0, 2))
                ),
                "user", nil, nil,
                list("tile", list("columns", 2, 4), list("rows", 0, 2))
            )
        )", R"(
            annotate_d([[3, 4], [-3, -4]], "tiled_array_2d_2_retiled/1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 2, 4), list("rows", 0, 2))))
        )");
    }
    else
    {
        test_retile_d_operation("test_retile_3loc2d_2", R"(
            retile_d(
                annotate_d([[2, 3, 4], [-2, -3, -4]], "tiled_array_2d_2",
                    list("tile", list("columns", 1, 4), list("rows", 0, 2))
                ),
                "user", nil, nil,
                list("tile", list("columns", 4, 6), list("rows", 0, 2))
            )
        )", R"(
            annotate_d([[5, 6], [-5, -6]], "tiled_array_2d_2_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 4, 6), list("rows", 0, 2))))
        )");
    }
}

void test_retile_3loc_2d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_3loc2d_3", R"(
            retile_d(
                annotate_d([[1, 2, 3, 4, 5]],
                    "tiled_array_2d_3",
                    list("tile", list("columns", 0, 5), list("rows", 0, 1))
                ),
                "sym", 1
            )
        )", R"(
            annotate_d([[1, 2, 3], [-1, -2, -3], [11, 12, 13]],
                "tiled_array_2d_3_retiled/1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 3), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_3loc2d_3", R"(
            retile_d(
                annotate_d([[-1, -2, -3, -4, -5]], "tiled_array_2d_3",
                    list("tile", list("columns", 0, 5), list("rows", 1, 2))
                ),
                "sym", 1
            )
        )", R"(
            annotate_d([[3, 4, 5], [-3, -4, -5], [13, 14, 15]],
                "tiled_array_2d_3_retiled/1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 2, 5), list("rows", 0, 3))))
        )");
    }
    else
    {
        test_retile_d_operation("test_retile_3loc2d_3", R"(
            retile_d(
                annotate_d([[11, 12, 13, 14, 15]], "tiled_array_2d_3",
                    list("tile", list("columns", 0, 5), list("rows", 2, 3))
                ),
                "sym", 1
            )
        )", R"(
            annotate_d([[4, 5], [-4, -5], [14, 15]],
                "tiled_array_2d_3_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 3, 5), list("rows", 0, 3))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_retile_3loc_1d_0();
    test_retile_3loc_1d_1();
    test_retile_3loc_1d_2();
    test_retile_3loc_1d_3();
    test_retile_3loc_1d_4();
    test_retile_3loc_1d_5();
    test_retile_3loc_1d_6();

    test_retile_3loc_2d_0();
    test_retile_3loc_2d_1();
    test_retile_3loc_2d_2();
    test_retile_3loc_2d_3();


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

