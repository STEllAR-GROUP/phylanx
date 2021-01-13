// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


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

void test_constant_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_constant_4loc_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_4loc1d_0", R"(
            constant_d(42, list(13), 0, 4)
        )", R"(
            annotate_d([42.0, 42.0, 42.0, 42.0], "full_array_1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test_constant_4loc1d_0", R"(
            constant_d(42, list(13), 1, 4)
        )", R"(
            annotate_d([42.0, 42.0, 42.0], "full_array_1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 4, 7))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test_constant_4loc1d_0", R"(
            constant_d(42, list(13), 2, 4)
        )", R"(
            annotate_d([42.0, 42.0, 42.0], "full_array_1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 7, 10))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_constant_d_operation("test_constant_4loc1d_0", R"(
            constant_d(42, list(13), 3, 4)
        )", R"(
            annotate_d([42.0, 42.0, 42.0], "full_array_1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 10, 13))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_constant_4loc_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_4loc2d_0", R"(
            constant_d(42, list(4, 7), 0, 4)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0], [42.0, 42.0, 42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 4), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test_constant_4loc2d_0", R"(
            constant_d(42, list(4, 7), 1, 4)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 4, 7), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test_constant_4loc2d_0", R"(
            constant_d(42, list(4, 7), 2, 4)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0], [42.0, 42.0, 42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 4), list("rows", 2, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_constant_d_operation("test_constant_4loc2d_0", R"(
            constant_d(42, list(4, 7), 3, 4)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 4, 7), list("rows", 2, 4))))
        )");
    }
}

void test_constant_4loc_2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_4loc2d_1", R"(
            constant_d(42, list(3, 9), 0, 4, "", "column")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0]],
                "full_array_3",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 3), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test_constant_4loc2d_1", R"(
            constant_d(42, list(3, 9), 1, 4, "", "column")
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0]],
                "full_array_3",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 3, 5), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test_constant_4loc2d_1", R"(
            constant_d(42, list(3, 9), 2, 4, "", "column")
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0]],
                "full_array_3",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 5, 7), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_constant_d_operation("test_constant_4loc2d_1", R"(
            constant_d(42, list(3, 9), 3, 4, "", "column")
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0]],
                "full_array_3",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 7, 9), list("rows", 0, 3))))
        )");
    }
}

void test_constant_4loc_2d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_4loc2d_2", R"(
            constant_d(42, list(4, 9), 0, 4, "tiled_matrix_1", "row")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                "tiled_matrix_1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 9), list("rows", 0, 1))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test_constant_4loc2d_2", R"(
            constant_d(42, list(4, 9), 1, 4, "tiled_matrix_1", "row")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                "tiled_matrix_1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 0, 9), list("rows", 1, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test_constant_4loc2d_2", R"(
            constant_d(42, list(4, 9), 2, 4, "tiled_matrix_1", "row")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                "tiled_matrix_1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 9), list("rows", 2, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_constant_d_operation("test_constant_4loc2d_2", R"(
            constant_d(42, list(4, 9), 3, 4, "tiled_matrix_1", "row")
        )", R"(
            annotate_d([[42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0, 42.0]],
                "tiled_matrix_1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 0, 9), list("rows", 3, 4))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_constant_4loc_3d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_4loc3d_0", R"(
            constant_d(42, list(3, 4, 5), nil, nil, "const3d", "row")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0, 42.0, 42.0]]],
                "const3d",
                list("tile", list("pages", 0, 3),
                    list("columns", 0, 5), list("rows", 0, 1)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test_constant_4loc3d_0", R"(
            constant_d(42, list(3, 4, 5), nil, nil, "const3d", "row")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0, 42.0, 42.0]]],
                "const3d",
                list("tile", list("pages", 0, 3),
                    list("columns", 0, 5), list("rows", 1, 2)))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test_constant_4loc3d_0", R"(
            constant_d(42, list(3, 4, 5), nil, nil, "const3d", "row")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0, 42.0, 42.0]]],
                "const3d",
                list("tile", list("pages", 0, 3),
                    list("columns", 0, 5), list("rows", 2, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_constant_d_operation("test_constant_4loc3d_0", R"(
            constant_d(42, list(3, 4, 5), nil, nil, "const3d", "row")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0, 42.0, 42.0]]],
                "const3d",
                list("tile", list("pages", 0, 3),
                    list("columns", 0, 5), list("rows", 3, 4)))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_constant_4loc_1d_0();

    test_constant_4loc_2d_0();
    test_constant_4loc_2d_1();
    test_constant_4loc_2d_2();

    test_constant_4loc_3d_0();

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

