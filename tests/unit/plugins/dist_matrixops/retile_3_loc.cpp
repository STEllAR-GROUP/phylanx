// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

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
                list("tile", list("rows", 0, 2))
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
                list("tile", list("rows", 2, 7))
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
                list("tile", list("rows", 7, 9))
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
                list("tile", list("columns", 0, 4))
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
                list("tile", list("columns", 4, 7))
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
                list("tile", list("columns", 7, 9))
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
                list("tile", list("columns", 0, 2))
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
                list("tile", list("columns", 2, 5))
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
                list("tile", list("columns", 5, 9))
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
                list("tile", list("columns", 3, 7))
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
                list("tile", list("columns", 7, 9))
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
                list("tile", list("columns", 0, 3))
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
                list("tile", list("columns", 4, 7))
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
                list("tile", list("columns", 0, 4))
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
                list("tile", list("columns", 7, 9))
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
                list("tile", list("columns", 4, 7))
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
                list("tile", list("columns", 7, 9))
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
                list("tile", list("columns", 0, 4))
            )
        )", R"(
            annotate_d([1, 2, 3, 4], "tiled_array_1d_5_retiled/1",
                list("args",
                    list("locality", 2, 3),
                    list("tile", list("columns", 0, 4))))
        )");
    }
}
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_retile_3loc_1d_0();
    test_retile_3loc_1d_1();
    test_retile_3loc_1d_2();
    test_retile_3loc_1d_3();
    test_retile_3loc_1d_4();
    test_retile_3loc_1d_5();


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

