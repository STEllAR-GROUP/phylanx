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
void test_retile_6loc_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_6loc1d_0", R"(
            retile_d(
                annotate_d([1, 2, 3, 4], "tiled_array_1d_0",
                    list("tile", list("columns", 0, 4))
                ),
                "user", nil, list("tile", list("rows", 15, 17))
            )
        )", R"(
            annotate_d([16, 17], "tiled_array_1d_0_retiled/1",
                list("args",
                    list("locality", 0, 6),
                    list("tile", list("rows", 15, 17))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_6loc1d_0", R"(
            retile_d(
                annotate_d([5, 6], "tiled_array_1d_0",
                    list("tile", list("columns", 4, 6))
                ),
                "user", nil, list("tile", list("rows", 13, 15))
            )
        )", R"(
            annotate_d([14, 15], "tiled_array_1d_0_retiled/1",
                list("args",
                    list("locality", 1, 6),
                    list("tile", list("rows", 13, 15))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_retile_d_operation("test_retile_6loc1d_0", R"(
            retile_d(
                annotate_d([7, 8, 9], "tiled_array_1d_0",
                    list("tile", list("columns", 6, 9))
                ),
                "user", nil, list("tile", list("rows", 0, 5))
            )
        )", R"(
            annotate_d([1, 2, 3, 4, 5], "tiled_array_1d_0_retiled/1",
                list("args",
                    list("locality", 2, 6),
                    list("tile", list("rows", 0, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_retile_d_operation("test_retile_6loc1d_0", R"(
            retile_d(
                annotate_d([10], "tiled_array_1d_0",
                    list("tile", list("columns", 9, 10))
                ),
                "user", nil, list("tile", list("rows", 10, 13))
            )
        )", R"(
            annotate_d([11, 12, 13], "tiled_array_1d_0_retiled/1",
                list("args",
                    list("locality", 3, 6),
                    list("tile", list("rows", 10, 13))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_retile_d_operation("test_retile_6loc1d_0", R"(
            retile_d(
                annotate_d([11, 12, 13, 14], "tiled_array_1d_0",
                    list("tile", list("columns", 10, 14))
                ),
                "user", nil, list("tile", list("rows", 17, 20))
            )
        )", R"(
            annotate_d([18, 19, 20], "tiled_array_1d_0_retiled/1",
                list("args",
                    list("locality", 4, 6),
                    list("tile", list("rows", 17, 20))))
        )");
    }
    else if (hpx::get_locality_id() == 5)
    {
        test_retile_d_operation("test_retile_6loc1d_0", R"(
            retile_d(
                annotate_d([15, 16, 17, 18, 19, 20], "tiled_array_1d_0",
                    list("tile", list("columns", 14, 20))
                ),
                "user", nil, list("tile", list("rows", 5, 10))
            )
        )", R"(
            annotate_d([6, 7, 8, 9, 10], "tiled_array_1d_0_retiled/1",
                list("args",
                    list("locality", 5, 6),
                    list("tile", list("rows", 5, 10))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_retile_6loc_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_retile_d_operation("test_retile_6loc2d_0", R"(
            retile_d(
                annotate_d([[-6], [16], [-16]], "tiled_array_2d_0",
                    list("tile", list("columns", 5, 6), list("rows", 1, 4))
                ),
                list("tile", list("rows", 0, 2), list("columns", 0, 2))
            )
        )", R"(
            annotate_d([[1, 2], [-1, -2]], "tiled_array_2d_0_retiled/1",
                list("args",
                    list("locality", 0, 6),
                    list("tile", list("rows", 0, 2), list("columns", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_retile_d_operation("test_retile_6loc2d_0", R"(
            retile_d(
                annotate_d([[1]], "tiled_array_2d_0",
                    list("tile", list("columns", 0, 1), list("rows", 0, 1))
                ),
                list("tile", list("rows", 0, 2), list("columns", 2, 4))
            )
        )", R"(
            annotate_d([[3, 4], [-3, -4]], "tiled_array_2d_0_retiled/1",
                list("args",
                    list("locality", 1, 6),
                    list("tile", list("rows", 0, 2), list("columns", 2, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_retile_d_operation("test_retile_6loc2d_0", R"(
            retile_d(
                annotate_d([[2, 3, 4, 5]], "tiled_array_2d_0",
                    list("tile", list("columns", 1, 5), list("rows", 0, 1))
                ),
                list("tile", list("rows", 0, 2), list("columns", 4, 6))
            )
        )", R"(
            annotate_d([[5, 6], [-5, -6]], "tiled_array_2d_0_retiled/1",
                list("args",
                    list("locality", 2, 6),
                    list("tile", list("rows", 0, 2), list("columns", 4, 6))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_retile_d_operation("test_retile_6loc2d_0", R"(
            retile_d(
                annotate_d([[6]], "tiled_array_2d_0",
                    list("tile", list("columns", 5, 6), list("rows", 0, 1))
                ),
                list("tile", list("rows", 2, 4), list("columns", 0, 2))
            )
        )", R"(
            annotate_d([[11, 12], [-11, -12]], "tiled_array_2d_0_retiled/1",
                list("args",
                    list("locality", 3, 6),
                    list("tile", list("rows", 2, 4), list("columns", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_retile_d_operation("test_retile_6loc2d_0", R"(
            retile_d(
                annotate_d([[-1], [11], [-11]], "tiled_array_2d_0",
                    list("tile", list("columns", 0, 1), list("rows", 1, 4))
                ),
                list("tile", list("rows", 2, 4), list("columns", 2, 4))
            )
        )", R"(
            annotate_d([[13, 14], [-13, -14]], "tiled_array_2d_0_retiled/1",
                list("args",
                    list("locality", 4, 6),
                    list("tile", list("rows", 2, 4), list("columns", 2, 4))))
        )");
    }
    else
    {
        test_retile_d_operation("test_retile_6loc2d_0", R"(
            retile_d(
                annotate_d([[-2, -3, -4, -5], [12, 13, 14, 15],
                        [-12, -13, -14, -15]],
                    "tiled_array_2d_0",
                    list("tile", list("columns", 1, 5), list("rows", 1, 4))
                ),
                list("tile", list("rows", 2, 4), list("columns", 4, 6))
            )
        )", R"(
            annotate_d([[15, 16], [-15, -16]], "tiled_array_2d_0_retiled/1",
                list("args",
                    list("locality", 5, 6),
                    list("tile", list("rows", 2, 4), list("columns", 4, 6))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_retile_6loc_1d_0();
    //test_retile_6loc_2d_0();


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

