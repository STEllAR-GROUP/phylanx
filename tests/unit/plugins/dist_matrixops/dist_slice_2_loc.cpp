// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

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

void test_slice_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_slice_column_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_slice_d_operation("test_slice_column_2loc_0", R"(
            slice_column(annotate_d([[1, 2, 3]], "array_0",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 3), list("rows", 0, 1)))),
            1)
        )", R"(
            annotate_d([2], "array_0_sliced/1",
                list("tile", list("columns", 0, 1)))
        )");
    }
    else
    {
        test_slice_d_operation("test_slice_column_2loc_0", R"(
            slice_column(annotate_d([[4, 5, 6], [7, 8, 9]], "array_0",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 3), list("rows", 1, 3)))),
            1)
        )", R"(
            annotate_d([5, 8], "array_0_sliced/1",
                list("tile", list("columns", 1, 3)))
        )");
    }
}

void test_slice_column_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_slice_d_operation("test_slice_column_2loc_1", R"(
            slice_column(annotate_d([[1, 2], [4, 5], [7, 8]], "array_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2), list("rows", 0, 3)))),
            1)
        )", R"(
            annotate_d([2, 5, 8], "array_1_sliced/1",
                list("tile", list("columns", 0, 3)))
        )");
    }
    else
    {
        test_slice_d_operation("test_slice_column_2loc_1", R"(
            slice_column(annotate_d([[3], [6], [9]], "array_1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 2, 3), list("rows", 0, 3)))),
            1)
        )", R"(
            annotate_d(nil, "array_1_sliced/1",
                list("tile", list("columns", 0, 0)))
        )");
    }
}

void test_slice_row_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_slice_d_operation("test_slice_row_2loc_0", R"(
            slice_row(annotate_d([[1, 2], [4, 5], [7, 8]], "array_2",
                    list("tile", list("columns", 0, 2), list("rows", 0, 3))),
            1)
        )", R"(
            annotate_d([4, 5], "array_2_sliced/1",
                list("tile", list("rows", 0, 2)))
        )");
    }
    else
    {
        test_slice_d_operation("test_slice_row_2loc_0", R"(
            slice_row(annotate_d([[3], [6], [9]], "array_2",
                    list("tile", list("columns", 2, 3), list("rows", 0, 3))),
            1)
        )", R"(
            annotate_d([6], "array_2_sliced/1",
                list("tile", list("rows", 2, 3)))
        )");
    }
}

void test_slice_row_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_slice_d_operation("test_slice_row_2loc_1", R"(
            slice_row(annotate_d([[1, 2, 3]], "array_3",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 3), list("rows", 0, 1)))),
            2)
        )", R"(
            annotate_d(nil, "array_3_sliced/1",
                list("tile", list("rows", 0, 0)))
        )");
    }
    else
    {
        test_slice_d_operation("test_slice_row_2loc_1", R"(
            slice_row(annotate_d([[4, 5, 6], [7, 8, 9]], "array_3",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 3), list("rows", 1, 3)))),
            2)
        )", R"(
            annotate_d([7, 8, 9], "array_3_sliced/1",
                list("tile", list("rows", 0, 3)))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_slice_column_0();
    test_slice_column_1();

    test_slice_row_0();
    test_slice_row_1();


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

