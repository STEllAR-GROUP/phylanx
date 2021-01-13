//   Copyright (c) 2020 Hartmut Kaiser
//   Copyright (c) 2020 Nanmiao Wu
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/iostream.hpp>
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
void test_all_gather_d_operation(std::string const& name,
    std::string const& code, std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

////////////////////////////////////////////////////////////////////////////////
void test_all_gather_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[1, 4], [2, 5], [3, 6]], "array_0",
                    list("tile", list("columns", 0, 2), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13], [2, 5, 8, 11, 14], [3, 6, 9, 12, 15]]");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[7], [8], [9]], "array_0",
                    list("tile", list("columns", 2, 3), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13], [2, 5, 8, 11, 14], [3, 6, 9, 12, 15]]");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[10], [11], [12]], "array_0",
                    list("tile", list("columns", 3, 4), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13], [2, 5, 8, 11, 14], [3, 6, 9, 12, 15]]");
    }
    else
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[13], [14], [15]], "array_0",
                    list("tile", list("columns", 4, 5), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13], [2, 5, 8, 11, 14], [3, 6, 9, 12, 15]]");
    }
}

void test_all_gather_2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_all_gather_d_operation("test2d_1", R"(
            all_gather_d(
                annotate_d([[1, 2, 3, 4], [5, 6, 7, 8]], "array_1",
                    list("tile", list("columns", 0, 4), list("rows", 0, 2))
                )
            )
        )", "[[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16],"
            "[17, 18, 19, 20]]");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_all_gather_d_operation("test2d_1", R"(
            all_gather_d(
                annotate_d([[9, 10, 11, 12]], "array_1",
                    list("tile", list("columns", 0, 4), list("rows", 2, 3))
                )
            )
        )", "[[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16],"
            "[17, 18, 19, 20]]");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_all_gather_d_operation("test2d_1", R"(
            all_gather_d(
                annotate_d([[13, 14, 15, 16]], "array_1",
                    list("tile", list("columns", 0, 4), list("rows", 3, 4))
                )
            )
        )", "[[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16],"
            "[17, 18, 19, 20]]");
    }
    else
    {
        test_all_gather_d_operation("test2d_1", R"(
            all_gather_d(
                annotate_d([[17, 18, 19, 20]], "array_1",
                    list("tile", list("columns", 0, 4), list("rows", 4, 5))
                )
            )
        )", "[[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], [13, 14, 15, 16],"
            "[17, 18, 19, 20]]");
    }
}

void test_all_gather_2d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_all_gather_d_operation("test2d_2", R"(
            all_gather_d(
                annotate_d([[1], [2], [3]], "array_2",
                    list("tile", list("columns", 0, 1), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13], [2, 5, 8, 11, 14], [3, 6, 9, 12, 15]]");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_all_gather_d_operation("test2d_2", R"(
            all_gather_d(
                annotate_d([[4], [5], [6]], "array_2",
                    list("tile", list("columns", 1, 2), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13], [2, 5, 8, 11, 14], [3, 6, 9, 12, 15]]");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_all_gather_d_operation("test2d_2", R"(
            all_gather_d(
                annotate_d([[7, 10], [8, 11], [9, 12]], "array_2",
                    list("tile", list("columns", 2, 4), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13], [2, 5, 8, 11, 14], [3, 6, 9, 12, 15]]");
    }
    else
    {
        test_all_gather_d_operation("test2d_2", R"(
            all_gather_d(
                annotate_d([[13], [14], [15]], "array_2",
                    list("tile", list("columns", 4, 5), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13], [2, 5, 8, 11, 14], [3, 6, 9, 12, 15]]");
    }
}

void test_all_gather_2d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_all_gather_d_operation("test2d_3", R"(
            all_gather_d(
                annotate_d([[1, 2, 3]], "array_3",
                    list("tile", list("columns", 0, 3), list("rows", 0, 1))
                )
            )
        )", "[[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12],"
            "[13, 14, 15], [16, 17, 18]]");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_all_gather_d_operation("test2d_3", R"(
            all_gather_d(
                annotate_d([[4, 5, 6], [7, 8, 9]], "array_3",
                    list("tile", list("columns", 0, 3), list("rows", 1, 3))
                )
            )
        )", "[[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12],"
            "[13, 14, 15], [16, 17, 18]]");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_all_gather_d_operation("test2d_3", R"(
            all_gather_d(
                annotate_d([[10, 11, 12]], "array_3",
                    list("tile", list("columns", 0, 3), list("rows", 3, 4))
                )
            )
        )", "[[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12],"
            "[13, 14, 15], [16, 17, 18]]");
    }
    else
    {
        test_all_gather_d_operation("test2d_3", R"(
            all_gather_d(
                annotate_d([[13, 14, 15], [16, 17, 18]], "array_3",
                    list("tile", list("columns", 0, 3), list("rows", 4, 6))
                )
            )
        )", "[[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12],"
            "[13, 14, 15], [16, 17, 18]]");
    }
}



////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_all_gather_2d_0();
    test_all_gather_2d_1();
    test_all_gather_2d_2();
    test_all_gather_2d_3();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    hpx::init_params params;
    params.cfg = std::move(cfg);
    return hpx::init(argc, argv, params);
}
