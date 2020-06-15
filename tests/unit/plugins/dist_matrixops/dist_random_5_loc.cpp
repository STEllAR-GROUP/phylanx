// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
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

void test_random_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    // comparing annotations
    HPX_TEST_EQ(hpx::cout, *(result.annotation()),*(comparison.annotation()));
}

///////////////////////////////////////////////////////////////////////////////
void test_random_5loc_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_random_d_operation("test_random_5loc2d_0", R"(
            random_d(list(5, 8), 0, 5, "rand58", "row")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                "rand58",
                list("args",
                    list("locality", 0, 5),
                    list("tile", list("columns", 0, 8), list("rows", 0, 1))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_random_d_operation("test_random_5loc2d_0", R"(
            random_d(list(5, 8), 1, 5, "rand58", "row")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                "rand58",
                list("args",
                    list("locality", 1, 5),
                    list("tile", list("columns", 0, 8), list("rows", 1, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_random_d_operation("test_random_5loc2d_0", R"(
            random_d(list(5, 8), 2, 5, "rand58", "row")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                "rand58",
                list("args",
                    list("locality", 2, 5),
                    list("tile", list("columns", 0, 8), list("rows", 2, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_random_d_operation("test_random_5loc2d_0", R"(
            random_d(list(5, 8), 3, 5, "rand58", "row")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                "rand58",
                list("args",
                    list("locality", 3, 5),
                    list("tile", list("columns", 0, 8), list("rows", 3, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_random_d_operation("test_random_5loc2d_0", R"(
            random_d(list(5, 8), 4, 5, "rand58", "row")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                "rand58",
                list("args",
                    list("locality", 4, 5),
                    list("tile", list("columns", 0, 8), list("rows", 4, 5))))
        )");
    }
}

void test_random_5loc_2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_random_d_operation("test_random_5loc2d_1", R"(
            random_d(list(5, 8), 0, 5)
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0], [42.0, 42.0],
                [42.0, 42.0]],
                "random_array_1",
                list("args",
                    list("locality", 0, 5),
                    list("tile", list("columns", 0, 2), list("rows", 0, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_random_d_operation("test_random_5loc2d_1", R"(
            random_d(list(5, 8), 1, 5)
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0], [42.0, 42.0],
                [42.0, 42.0]],
                "random_array_1",
                list("args",
                    list("locality", 1, 5),
                    list("tile", list("columns", 2, 4), list("rows", 0, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_random_d_operation("test_random_5loc2d_1", R"(
            random_d(list(5, 8), 2, 5)
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0], [42.0, 42.0],
                [42.0, 42.0]],
                "random_array_1",
                list("args",
                    list("locality", 2, 5),
                    list("tile", list("columns", 4, 6), list("rows", 0, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_random_d_operation("test_random_5loc2d_1", R"(
            random_d(list(5, 8), 3, 5)
        )", R"(
            annotate_d([[42.0], [42.0], [42.0], [42.0], [42.0]],
                "random_array_1",
                list("args",
                    list("locality", 3, 5),
                    list("tile", list("columns", 6, 7), list("rows", 0, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_random_d_operation("test_random_5loc2d_1", R"(
            random_d(list(5, 8), 4, 5)
        )", R"(
            annotate_d([[42.0], [42.0], [42.0], [42.0], [42.0]],
                "random_array_1",
                list("args",
                    list("locality", 4, 5),
                    list("tile", list("columns", 7, 8), list("rows", 0, 5))))
        )");
    }
}

void test_random_5loc_3d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_random_d_operation("test_random_5loc3d_0", R"(
            random_d(list(6, 1, 8), 0, 5, "rand618", "page")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]]],
                "rand618",
                list("tile", list("pages", 0, 2),
                    list("columns", 0, 8), list("rows", 0, 1)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_random_d_operation("test_random_5loc3d_0", R"(
            random_d(list(6, 1, 8), 1, 5, "rand618", "page")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]]],
                "rand618",
                list("tile", list("pages", 2, 3),
                    list("columns", 0, 8), list("rows", 0, 1)))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_random_d_operation("test_random_5loc3d_0", R"(
            random_d(list(6, 1, 8), 2, 5, "rand618", "page")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]]],
                "rand618",
                list("tile", list("pages", 3, 4),
                    list("columns", 0, 8), list("rows", 0, 1)))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_random_d_operation("test_random_5loc3d_0", R"(
            random_d(list(6, 1, 8), 3, 5, "rand618", "page")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]]],
                "rand618",
                list("tile", list("pages", 4, 5),
                    list("columns", 0, 8), list("rows", 0, 1)))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_random_d_operation("test_random_5loc3d_0", R"(
            random_d(list(6, 1, 8), 4, 5, "rand618", "page")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]]],
                "rand618",
                list("tile", list("pages", 5, 6),
                    list("columns", 0, 8), list("rows", 0, 1)))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    // only annotations are compared
    test_random_5loc_2d_0();
    test_random_5loc_2d_1();
    test_random_5loc_3d_0();

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

