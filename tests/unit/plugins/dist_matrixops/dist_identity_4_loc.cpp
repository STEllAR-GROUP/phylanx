// Copyright (c) 2020 Nanmiao Wu
// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2017-2020 Hartmut Kaiser

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
void test_identity_4loc_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_4loc_0", R"(
            identity_d(4, 0, 4, "", "column")
        )",
            R"(
            annotate_d([[1.0], [0.0], [0.0], [0.0]],
                "identity_array_1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 1), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_identity_d_operation("test_identity_4loc_0", R"(
            identity_d(4, 1, 4, "", "column")
        )",
            R"(
            annotate_d([[0.0], [1.0], [0.0], [0.0]],
                "identity_array_1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 1, 2), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_identity_d_operation("test_identity_4loc_0", R"(
            identity_d(4, 2, 4, "", "column")
        )",
            R"(
            annotate_d([[0.0], [0.0], [1.0], [0.0]],
                "identity_array_1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 2, 3), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_identity_d_operation("test_identity_4loc_0", R"(
            identity_d(4, 3, 4, "", "column")
        )",
            R"(
            annotate_d([[0.0], [0.0], [0.0], [1.0]],
                "identity_array_1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 3, 4), list("rows", 0, 4))))
        )");
    }
}

void test_identity_4loc_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_4loc_1", R"(
            identity_d(4, 0, 4, "my_identity_1", "column")
        )",
            R"(
            annotate_d([[1.0], [0.0], [0.0], [0.0]],
                "my_identity_1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 1), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_identity_d_operation("test_identity_4loc_1", R"(
            identity_d(4, 1, 4, "my_identity_1", "column")
        )",
            R"(
            annotate_d([[0.0], [1.0], [0.0], [0.0]],
                "my_identity_1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 1, 2), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_identity_d_operation("test_identity_4loc_1", R"(
            identity_d(4, 2, 4, "my_identity_1", "column")
        )",
            R"(
            annotate_d([[0.0], [0.0], [1.0], [0.0]],
                "my_identity_1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 2, 3), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_identity_d_operation("test_identity_4loc_1", R"(
            identity_d(4, 3, 4, "my_identity_1", "column")
        )",
            R"(
            annotate_d([[0.0], [0.0], [0.0], [1.0]],
                "my_identity_1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 3, 4), list("rows", 0, 4))))
        )");
    }
}

void test_identity_4loc_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_4loc_2", R"(
            identity_d(4, 0, 4, "my_identity_2", "row")
        )",
            R"(
            annotate_d([[1.0, 0.0, 0.0, 0.0]],
                "my_identity_2",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 4), list("rows", 0, 1))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_identity_d_operation("test_identity_4loc_2", R"(
            identity_d(4, 1, 4, "my_identity_2", "row")
        )",
            R"(
            annotate_d([[0.0, 1.0, 0.0, 0.0]],
                "my_identity_2",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 0, 4), list("rows", 1, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_identity_d_operation("test_identity_4loc_2", R"(
            identity_d(4, 2, 4, "my_identity_2", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 1.0, 0.0]],
                "my_identity_2",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 4), list("rows", 2, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_identity_d_operation("test_identity_4loc_2", R"(
            identity_d(4, 3, 4, "my_identity_2", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 0.0, 1.0]],
                "my_identity_2",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 0, 4), list("rows", 3, 4))))
        )");
    }
}

void test_identity_4loc_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_4loc_3", R"(
            identity_d(4, 0, 4, "", "row")
        )",
            R"(
            annotate_d([[1.0, 0.0, 0.0, 0.0]],
                "identity_array_2",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 4), list("rows", 0, 1))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_identity_d_operation("test_identity_4loc_3", R"(
            identity_d(4, 1, 4, "", "row")
        )",
            R"(
            annotate_d([[0.0, 1.0, 0.0, 0.0]],
                "identity_array_2",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 0, 4), list("rows", 1, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_identity_d_operation("test_identity_4loc_3", R"(
            identity_d(4, 2, 4, "", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 1.0, 0.0]],
                "identity_array_2",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 4), list("rows", 2, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_identity_d_operation("test_identity_4loc_3", R"(
            identity_d(4, 3, 4, "", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 0.0, 1.0]],
                "identity_array_2",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 0, 4), list("rows", 3, 4))))
        )");
    }
}

void test_identity_4loc_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_4loc_4", R"(
            identity_d(4, 0, 4, "my_identity_3", "sym")
        )",
            R"(
            annotate_d([[1.0, 0.0], [0.0, 1.0]],
                "my_identity_3",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 2), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_identity_d_operation("test_identity_4loc_4", R"(
            identity_d(4, 1, 4, "my_identity_3", "sym")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0]],
                "my_identity_3",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 2, 4), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_identity_d_operation("test_identity_4loc_4", R"(
            identity_d(4, 2, 4, "my_identity_3", "sym")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0]],
                "my_identity_3",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 2), list("rows", 2, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_identity_d_operation("test_identity_4loc_4", R"(
            identity_d(4, 3, 4, "my_identity_3", "sym")
        )",
            R"(
            annotate_d([[1.0, 0.0], [0.0, 1.0]],
                "my_identity_3",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 2, 4), list("rows", 2, 4))))
        )");
    }
}

void test_identity_4loc_5()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_4loc_5", R"(
            identity_d(4, 0, 4, "", "sym")
        )",
            R"(
            annotate_d([[1.0, 0.0], [0.0, 1.0]],
                "identity_array_3",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 2), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_identity_d_operation("test_identity_4loc_5", R"(
            identity_d(4, 1, 4, "", "sym")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0]],
                "identity_array_3",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 2, 4), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_identity_d_operation("test_identity_4loc_5", R"(
            identity_d(4, 2, 4, "", "sym")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0]],
                "identity_array_3",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 2), list("rows", 2, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_identity_d_operation("test_identity_4loc_5", R"(
            identity_d(4, 3, 4, "", "sym")
        )",
            R"(
            annotate_d([[1.0, 0.0], [0.0, 1.0]],
                "identity_array_3",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 2, 4), list("rows", 2, 4))))
        )");
    }
}
///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_identity_4loc_0();
    test_identity_4loc_1();
    test_identity_4loc_2();
    test_identity_4loc_3();
    test_identity_4loc_4();
    test_identity_4loc_5();

    hpx::finalize();
    return hpx::util::report_errors();
}
int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
