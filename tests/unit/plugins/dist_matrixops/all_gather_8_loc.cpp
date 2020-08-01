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
        )", "[[1, 4, 7, 10, 13, 16, 19, 22, 25],"
            "[2, 5, 8, 11, 14, 17, 20, 23, 26],"
            "[3, 6, 9, 12, 15, 18, 21, 24, 27]]");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[7], [8], [9]], "array_0",
                    list("tile", list("columns", 2, 3), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13, 16, 19, 22, 25],"
            "[2, 5, 8, 11, 14, 17, 20, 23, 26],"
            "[3, 6, 9, 12, 15, 18, 21, 24, 27]]");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[10], [11], [12]], "array_0",
                    list("tile", list("columns", 3, 4), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13, 16, 19, 22, 25],"
            "[2, 5, 8, 11, 14, 17, 20, 23, 26],"
            "[3, 6, 9, 12, 15, 18, 21, 24, 27]]");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[13], [14], [15]], "array_0",
                    list("tile", list("columns", 4, 5), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13, 16, 19, 22, 25],"
            "[2, 5, 8, 11, 14, 17, 20, 23, 26],"
            "[3, 6, 9, 12, 15, 18, 21, 24, 27]]");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[16], [17], [18]], "array_0",
                    list("tile", list("columns", 5, 6), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13, 16, 19, 22, 25],"
            "[2, 5, 8, 11, 14, 17, 20, 23, 26],"
            "[3, 6, 9, 12, 15, 18, 21, 24, 27]]");
    }
    else if (hpx::get_locality_id() == 5)
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[19], [20], [21]], "array_0",
                    list("tile", list("columns", 6, 7), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13, 16, 19, 22, 25],"
            "[2, 5, 8, 11, 14, 17, 20, 23, 26],"
            "[3, 6, 9, 12, 15, 18, 21, 24, 27]]");
    }
    else if (hpx::get_locality_id() == 6)
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[22], [23], [24]], "array_0",
                    list("tile", list("columns", 7, 8), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13, 16, 19, 22, 25],"
            "[2, 5, 8, 11, 14, 17, 20, 23, 26],"
            "[3, 6, 9, 12, 15, 18, 21, 24, 27]]");
    }
    else
    {
        test_all_gather_d_operation("test2d_0", R"(
            all_gather_d(
                annotate_d([[25], [26], [27]], "array_0",
                    list("tile", list("columns", 8, 9), list("rows", 0, 3))
                )
            )
        )", "[[1, 4, 7, 10, 13, 16, 19, 22, 25],"
            "[2, 5, 8, 11, 14, 17, 20, 23, 26],"
            "[3, 6, 9, 12, 15, 18, 21, 24, 27]]");
    }
}





////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_all_gather_2d_0();

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
