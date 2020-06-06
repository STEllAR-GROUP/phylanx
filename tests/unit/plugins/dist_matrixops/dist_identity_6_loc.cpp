// Copyright (c) 2020 Nanmiao Wu
// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2017-2020 Hartmut Kaiser

//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
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

void test_identity_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_identity_6loc_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_6loc_0", R"(
            identity_d(7, 0, 6, "", "sym")
        )",
            R"(
            annotate_d([[1.0, 0.0, 0.0], [0.0, 1.0, 0.0],
                [0.0, 0.0, 1.0], [0.0, 0.0, 0.0]],
                "identity_array_1",
                list("args",
                    list("locality", 0, 6),
                    list("tile", list("columns", 0, 3), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_identity_d_operation("test_identity_6loc_0", R"(
            identity_d(7, 1, 6, "", "sym")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0],
                [0.0, 0.0], [1.0, 0.0]],
                "identity_array_1",
                list("args",
                    list("locality", 1, 6),
                    list("tile", list("columns", 3, 5), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_identity_d_operation("test_identity_6loc_0", R"(
            identity_d(7, 2, 6, "", "sym")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0],
                [0.0, 0.0], [0.0, 0.0]],
                "identity_array_1",
                list("args",
                    list("locality", 2, 6),
                    list("tile", list("columns", 5, 7), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_identity_d_operation("test_identity_6loc_0", R"(
            identity_d(7, 3, 6, "", "sym")
        )",
            R"(
            annotate_d([[0.0, 0.0, 0.0], [0.0, 0.0, 0.0],
                [0.0, 0.0, 0.0]],
                "identity_array_1",
                list("args",
                    list("locality", 3, 6),
                    list("tile", list("columns", 0, 3), list("rows", 4, 7))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_identity_d_operation("test_identity_6loc_0", R"(
            identity_d(7, 4, 6, "", "sym")
        )",
            R"(
            annotate_d([[0.0, 1.0], [0.0, 0.0],
                [0.0, 0.0]],
                "identity_array_1",
                list("args",
                    list("locality", 4, 6),
                    list("tile", list("columns", 3, 5), list("rows", 4, 7))))
        )");
    }
    else if (hpx::get_locality_id() == 5)
    {
        test_identity_d_operation("test_identity_6loc_0", R"(
            identity_d(7, 5, 6, "", "sym")
        )",
            R"(
            annotate_d([[0.0, 0.0], [1.0, 0.0],
                [0.0, 1.0]],
                "identity_array_1",
                list("args",
                    list("locality", 5, 6),
                    list("tile", list("columns", 5, 7), list("rows", 4, 7))))
        )");
    }
}

void test_identity_6loc_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_6loc_1", R"(
            identity_d(8, 0, 6, "my_identity_1", "row")
        )",
            R"(
            annotate_d([[1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]],
                "my_identity_1",
                list("args",
                    list("locality", 0, 6),
                    list("tile", list("columns", 0, 8), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_identity_d_operation("test_identity_6loc_1", R"(
            identity_d(8, 1, 6, "my_identity_1", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                [0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0]],
                "my_identity_1",
                list("args",
                    list("locality", 1, 6),
                    list("tile", list("columns", 0, 8), list("rows", 2, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_identity_d_operation("test_identity_6loc_1", R"(
            identity_d(8, 2, 6, "my_identity_1", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0]],
                "my_identity_1",
                list("args",
                    list("locality", 2, 6),
                    list("tile", list("columns", 0, 8), list("rows", 4, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_identity_d_operation("test_identity_6loc_1", R"(
            identity_d(8, 3, 6, "my_identity_1", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0]],
                "my_identity_1",
                list("args",
                    list("locality", 3, 6),
                    list("tile", list("columns", 0, 8), list("rows", 5, 6))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_identity_d_operation("test_identity_6loc_1", R"(
            identity_d(8, 4, 6, "my_identity_1", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0]],
                "my_identity_1",
                list("args",
                    list("locality", 4, 6),
                    list("tile", list("columns", 0, 8), list("rows", 6, 7))))
        )");
    }
    else if (hpx::get_locality_id() == 5)
    {
        test_identity_d_operation("test_identity_6loc_1", R"(
            identity_d(8, 5, 6, "my_identity_1", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0]],
                "my_identity_1",
                list("args",
                    list("locality", 5, 6),
                    list("tile", list("columns", 0, 8), list("rows", 7, 8))))
        )");
    }
}

void test_identity_6loc_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_6loc_2", R"(
            identity_d(9, 0, 6, "my_identity_2", "column")
        )",
            R"(
            annotate_d([[1.0, 0.0], [0.0, 1.0], [0.0, 0.0], [0.0, 0.0],
                [0.0, 0.0], [0.0, 0.0],[0.0, 0.0], [0.0, 0.0],[0.0, 0.0]],
                "my_identity_2",
                list("args",
                    list("locality", 0, 6),
                    list("tile", list("columns", 0, 2), list("rows", 0, 9))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_identity_d_operation("test_identity_6loc_2", R"(
            identity_d(9, 1, 6, "my_identity_2", "column")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0], [1.0, 0.0], [0.0, 1.0],
                [0.0, 0.0], [0.0, 0.0],[0.0, 0.0], [0.0, 0.0],[0.0, 0.0]],
                "my_identity_2",
                list("args",
                    list("locality", 1, 6),
                    list("tile", list("columns", 2, 4), list("rows", 0, 9))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_identity_d_operation("test_identity_6loc_2", R"(
            identity_d(9, 2, 6, "my_identity_2", "column")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0], [0.0, 0.0], [0.0, 0.0],
                [1.0, 0.0], [0.0, 1.0],[0.0, 0.0], [0.0, 0.0],[0.0, 0.0]],
                "my_identity_2",
                list("args",
                    list("locality", 2, 6),
                    list("tile", list("columns", 4, 6), list("rows", 0, 9))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_identity_d_operation("test_identity_6loc_2", R"(
            identity_d(9, 3, 6, "my_identity_2", "column")
        )",
            R"(
            annotate_d([[0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [1.0],
                [0.0], [0.0]],
                "my_identity_2",
                list("args",
                    list("locality", 3, 6),
                    list("tile", list("columns", 6, 7), list("rows", 0, 9))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_identity_d_operation("test_identity_6loc_2", R"(
            identity_d(9, 4, 6, "my_identity_2", "column")
        )",
            R"(
            annotate_d([[0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0],
                [1.0], [0.0]],
                "my_identity_2",
                list("args",
                    list("locality", 4, 6),
                    list("tile", list("columns", 7, 8), list("rows", 0, 9))))
        )");
    }
    else if (hpx::get_locality_id() == 5)
    {
        test_identity_d_operation("test_identity_6loc_2", R"(
            identity_d(9, 5, 6, "my_identity_2", "column")
        )",
            R"(
            annotate_d([[0.0], [0.0], [0.0], [0.0], [0.0], [0.0], [0.0],
                [0.0], [1.0]],
                "my_identity_2",
                list("args",
                    list("locality", 5, 6),
                    list("tile", list("columns", 8, 9), list("rows", 0, 9))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_identity_6loc_0();
    test_identity_6loc_1();
    test_identity_6loc_2();

    hpx::finalize();
    return hpx::util::report_errors();
}
int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
