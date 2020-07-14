// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/distributed/iostream.hpp>
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
void test_constant_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_2loc1d_0", R"(
            constant_d(42, list(4), 0, 2)
        )", R"(
            annotate_d([42.0, 42.0], "full_array_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2))))
        )");
    }
    else
    {
        test_constant_d_operation("test_constant_2loc1d_0", R"(
            constant_d(42, list(4), 1, 2)
        )", R"(
            annotate_d([42.0, 42.0], "full_array_1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 2, 4))))
        )");
    }
}

void test_constant_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_2loc1d_1", R"(
            constant_d(42, list(5), 0, 2)
        )", R"(
            annotate_d([42.0, 42.0, 42.0], "full_array_2",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 3))))
        )");
    }
    else
    {
        test_constant_d_operation("test_constant_2loc1d_1", R"(
            constant_d(42, list(5), 1, 2)
        )", R"(
            annotate_d([42.0, 42.0], "full_array_2",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 3, 5))))
        )");
    }
}

void test_constant_1d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_2loc1d_2", R"(
            constant_d(13.0, list(7), 0, 2, "my_const_13")
        )", R"(
            annotate_d([13.0, 13.0, 13.0, 13.0], "my_const_13",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 4))))
        )");
    }
    else
    {
        test_constant_d_operation("test_constant_2loc1d_2", R"(
            constant_d(13.0, list(7), 1, 2, "my_const_13")
        )", R"(
            annotate_d([13.0, 13.0, 13.0], "my_const_13",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 4, 7))))
        )");
    }
}

void test_constant_1d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test1d_3", R"(
            constant_d(13.0, list(7), 0, 2, "my_const_13_2", "sym",
            __arg(dtype, "int"))
        )", R"(
            annotate_d([13, 13, 13, 13], "my_const_13_2",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 4))))
        )");
    }
    else
    {
        test_constant_d_operation("test1d_3", R"(
            constant_d(13.0, list(7), 1, 2, "my_const_13_2", "sym",
            __arg(dtype, "int"))
        )", R"(
            annotate_d([13, 13, 13], "my_const_13_2",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 4, 7))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_constant_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_2loc2d_0", R"(
            constant_d(42, list(4, 6), 0, 2)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                "full_array_3",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 3), list("rows", 0, 4))))
        )");
    }
    else
    {
        test_constant_d_operation("test_constant_2loc2d_0", R"(
            constant_d(42, list(4, 6), 1, 2)
        )", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                "full_array_3",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 3, 6), list("rows", 0, 4))))
        )");
    }
}

void test_constant_2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_2loc2d_1", R"(
            constant_d(42, list(5, 4), 0, 2, "const_42", "column")
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0],
                        [42.0, 42.0], [42.0, 42.0]],
                "const_42",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2), list("rows", 0, 5))))
        )");
    }
    else
    {
        test_constant_d_operation("test_constant_2loc2d_1", R"(
            constant_d(42, list(5, 4), 1, 2, "const_42", "column")
        )", R"(
            annotate_d([[42.0, 42.0], [42.0, 42.0], [42.0, 42.0],
                        [42.0, 42.0], [42.0, 42.0]],
                "const_42",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 2, 4), list("rows", 0, 5))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_constant_3d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_2loc3d_0", R"(
            constant_d(42, list(2, 4, 5), 0, 2, "", "column")
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0], [42.0, 42.0, 42.0]]],
                "full_array_4",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("pages", 0, 2),
                        list("columns", 0, 3), list("rows", 0, 4))))
        )");
    }
    else
    {
        test_constant_d_operation("test_constant_2loc3d_0", R"(
            constant_d(42, list(2, 4, 5), 1, 2, "", "column")
        )", R"(
            annotate_d([[[42.0, 42.0], [42.0, 42.0],
                        [42.0, 42.0], [42.0, 42.0]],
                        [[42.0, 42.0], [42.0, 42.0],
                        [42.0, 42.0], [42.0, 42.0]]],
                "full_array_4",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("pages", 0, 2),
                        list("columns", 3, 5), list("rows", 0, 4))))
        )");
    }
}

void test_constant_3d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_2loc3d_1", R"(
            constant_d(42, list(2, 4, 5))
        )", R"(
            annotate_d([[[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                        [[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0], [42.0, 42.0, 42.0]]],
                "full_array_5",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("pages", 0, 2),
                        list("columns", 0, 3), list("rows", 0, 4))))
        )");
    }
    else
    {
        test_constant_d_operation("test_constant_2loc3d_1", R"(
            constant_d(42, list(2, 4, 5))
        )", R"(
            annotate_d([[[42.0, 42.0], [42.0, 42.0],
                        [42.0, 42.0], [42.0, 42.0]],
                        [[42.0, 42.0], [42.0, 42.0],
                        [42.0, 42.0], [42.0, 42.0]]],
                "full_array_5",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("pages", 0, 2),
                        list("columns", 3, 5), list("rows", 0, 4))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_constant_1d_0();
    test_constant_1d_1();
    test_constant_1d_2();
    test_constant_1d_3();

    test_constant_2d_0();
    test_constant_2d_1();

    test_constant_3d_0();
    test_constant_3d_1();

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

