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

    // comparing annotations
    HPX_TEST_EQ(*(result.annotation()), *(comparison.annotation()));
}

///////////////////////////////////////////////////////////////////////////////
void test_identity_2loc_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_2loc_0", R"(
            identity_d(4, 0, 2, "my_identity_1", "column")
        )",
            R"(
            annotate_d([[1.0, 0.0], [0.0, 1.0], [0.0, 0.0],
                [0.0, 0.0]],
                "my_identity_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2), list("rows", 0, 4))))
        )");
    }
    else
    {
        test_identity_d_operation("test_identity_2loc_0", R"(
            identity_d(4, 1, 2, "my_identity_1", "column")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0], [1.0, 0.0],
                [0.0, 1.0]],
                "my_identity_1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 2, 4), list("rows", 0, 4))))
        )");
    }
}

void test_identity_2loc_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_2loc_1", R"(
            identity_d(4, 0, 2, "my_identity_2", "row")
        )",
            R"(
            annotate_d([[1.0, 0.0, 0.0, 0.0], [0.0, 1.0, 0.0, 0.0]],
                "my_identity_2",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 4), list("rows", 0, 2))))
        )");
    }
    else
    {
        test_identity_d_operation("test_identity_2loc_0", R"(
            identity_d(4, 1, 2, "my_identity_2", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 1.0, 0.0], [0.0, 0.0, 0.0, 1.0]],
                "my_identity_2",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 4), list("rows", 2, 4))))
        )");
    }
}

void test_identity_2loc_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_2loc_2", R"(
            identity_d(5, 0, 2, "my_identity_3", "column")
        )",
            R"(
            annotate_d([[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0],
                [0.0, 0.0, 0.0], [0.0, 0.0, 0.0]],
                "my_identity_3",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 3), list("rows", 0, 5))))
        )");
    }
    else
    {
        test_identity_d_operation("test_identity_2loc_2", R"(
            identity_d(5, 1, 2, "my_identity_3", "column")
        )",
            R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0], [0.0, 0.0], [1.0, 0.0],
                [0.0, 1.0]],
                "my_identity_3",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 3, 5), list("rows", 0, 5))))
        )");
    }
}

void test_identity_2loc_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_2loc_3", R"(
            identity_d(5, 0, 2, "my_identity_4", "row")
        )",
            R"(
            annotate_d([[1.0, 0.0, 0.0, 0.0, 0.0], [0.0, 1.0, 0.0, 0.0, 0.0],
                [0.0, 0.0, 1.0, 0.0, 0.0]],
                "my_identity_4",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 5), list("rows", 0, 3))))
        )");
    }
    else
    {
        test_identity_d_operation("test_identity_2loc_3", R"(
            identity_d(5, 1, 2, "my_identity_4", "row")
        )",
            R"(
            annotate_d([[0.0, 0.0, 0.0, 1.0, 0.0], [0.0, 0.0, 0.0, 0.0, 1.0]],
                "my_identity_4",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 5), list("rows", 3, 5))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    // only annotations are compared
    test_identity_2loc_0();
    test_identity_2loc_1();
    test_identity_2loc_2();
    test_identity_2loc_3();

    hpx::finalize();
    return hpx::util::report_errors();
}
int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
