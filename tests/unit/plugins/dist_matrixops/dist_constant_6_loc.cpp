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
void test_constant_6loc_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_6loc2d_0", R"(
            constant_d(42, list(5, 8), 0, 6)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0]],
                "full_array_1",
                list("args",
                    list("locality", 0, 6),
                    list("tile", list("columns", 0, 3), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test_constant_6loc2d_0", R"(
            constant_d(42, list(5, 8), 1, 6)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0]],
                "full_array_1",
                list("args",
                    list("locality", 1, 6),
                    list("tile", list("columns", 3, 6), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test_constant_6loc2d_0", R"(
            constant_d(42, list(5, 8), 2, 6)
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0]],
                "full_array_1",
                list("args",
                    list("locality", 2, 6),
                    list("tile", list("columns", 6, 8), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_constant_d_operation("test_constant_6loc2d_0", R"(
            constant_d(42, list(5, 8), 3, 6)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                "full_array_1",
                list("args",
                    list("locality", 3, 6),
                    list("tile", list("columns", 0, 3), list("rows", 3, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_constant_d_operation("test_constant_6loc2d_0", R"(
            constant_d(42, list(5, 8), 4, 6)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                "full_array_1",
                list("args",
                    list("locality", 4, 6),
                    list("tile", list("columns", 3, 6), list("rows", 3, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 5)
    {
        test_constant_d_operation("test_constant_6loc2d_0", R"(
            constant_d(42, list(5, 8), 5, 6)
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0]],
                "full_array_1",
                list("args",
                    list("locality", 5, 6),
                    list("tile", list("columns", 6, 8), list("rows", 3, 5))))
        )");
    }
}

void test_constant_6loc_2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_6loc_2d_1", R"(
            constant_d(42, list(8, 5), 0, 6)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 0, 6),
                    list("tile", list("columns", 0, 3), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test_constant_6loc_2d_1", R"(
            constant_d(42, list(8, 5), 1, 6)
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 1, 6),
                    list("tile", list("columns", 3, 5), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test_constant_6loc_2d_1", R"(
            constant_d(42, list(8, 5), 2, 6)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 2, 6),
                    list("tile", list("columns", 0, 3), list("rows", 3, 6))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_constant_d_operation("test_constant_6loc_2d_1", R"(
            constant_d(42, list(8, 5), 3, 6)
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 3, 6),
                    list("tile", list("columns", 3, 5), list("rows", 3, 6))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_constant_d_operation("test_constant_6loc_2d_1", R"(
            constant_d(42, list(8, 5), 4, 6)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 4, 6),
                    list("tile", list("columns", 0, 3), list("rows", 6, 8))))
        )");
    }
    else if (hpx::get_locality_id() == 5)
    {
        test_constant_d_operation("test_constant_6loc_2d_1", R"(
            constant_d(42, list(8, 5), 5, 6)
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0]],
                "full_array_2",
                list("args",
                    list("locality", 5, 6),
                    list("tile", list("columns", 3, 5), list("rows", 6, 8))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_constant_6loc_3d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_6loc3d_0", R"(
            constant_d(42, list(2, 5, 7))
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                         [42.0, 42.0, 42.0]],[[42.0, 42.0, 42.0],
                         [42.0, 42.0, 42.0], [42.0, 42.0, 42.0]]],
                "full_array_3",
                list("args",
                    list("locality", 0, 6),
                    list("tile", list("pages", 0, 2),
                        list("columns", 0, 3), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test_constant_6loc3d_0", R"(
            constant_d(42, list(2, 5, 7))
        )", R"(
            annotate_d([[[42.0, 42.0], [42.0, 42.0],
                         [42.0, 42.0]],[[42.0, 42.0],
                         [42.0, 42.0], [42.0, 42.0]]],
                "full_array_3",
                list("args",
                    list("locality", 1, 6),
                    list("tile", list("pages", 0, 2),
                        list("columns", 3, 5), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test_constant_6loc3d_0", R"(
            constant_d(42, list(2, 5, 7))
        )", R"(
            annotate_d([[[42.0, 42.0], [42.0, 42.0],
                         [42.0, 42.0]],[[42.0, 42.0],
                         [42.0, 42.0], [42.0, 42.0]]],
                "full_array_3",
                list("args",
                    list("locality", 2, 6),
                    list("tile", list("pages", 0, 2),
                        list("columns", 5, 7), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_constant_d_operation("test_constant_6loc3d_0", R"(
            constant_d(42, list(2, 5, 7))
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0], [42.0, 42.0, 42.0]]],
                "full_array_3",
                list("args",
                    list("locality", 3, 6),
                    list("tile", list("pages", 0, 2),
                        list("columns", 0, 3), list("rows", 3, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_constant_d_operation("test_constant_6loc3d_0", R"(
            constant_d(42, list(2, 5, 7))
        )", R"(
            annotate_d([[[42.0, 42.0], [42.0, 42.0]],
                        [[42.0, 42.0], [42.0, 42.0]]],
                "full_array_3",
                list("args",
                    list("locality", 4, 6),
                    list("tile", list("pages", 0, 2),
                        list("columns", 3, 5), list("rows", 3, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 5)
    {
        test_constant_d_operation("test_constant_6loc3d_0", R"(
            constant_d(42, list(2, 5, 7))
        )", R"(
            annotate_d([[[42.0, 42.0], [42.0, 42.0]],
                        [[42.0, 42.0], [42.0, 42.0]]],
                "full_array_3",
                list("args",
                    list("locality", 5, 6),
                    list("tile", list("pages", 0, 2),
                        list("columns", 5, 7), list("rows", 3, 5))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_constant_6loc_2d_0();
    test_constant_6loc_2d_1();

    test_constant_6loc_3d_0();

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

