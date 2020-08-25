// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
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

#include <blaze/Math.h>

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

void test_read_csv_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_read_csv_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_read_csv_d_operation("test_read_csv_2loc2d_0", R"(
            file_read_csv_d(
                "test_20200713_2loc",
                false,
                1,
                "sym",
                2
            )
        )", R"(
            annotate_d([[22, 30], [17.99, 10.38], [20.57, 17.77],
                [19.69, 21.25], [11.42, 20.38], [20.29, 14.34], [12.45, 15.7],
                [18.25, 19.98], [13.71, 20.83], [13, 21.82], [12.46, 24.04],
                [16.02, 23.24], [15.78, 17.89], [19.17, 24.8], [15.85, 23.95],
                [13.73, 22.61], [14.54, 27.54]],
                "csv_file_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2), list("rows", 0, 17))))
        )");
    }
    else
    {
        test_read_csv_d_operation("test_read_csv_2loc2d_0", R"(
            file_read_csv_d(
                "test_20200713_2loc",
                false,
                1,
                "sym",
                2
            )
        )", R"(
            annotate_d([[19.17, 24.8], [15.85, 23.95], [13.73, 22.61],
                [14.54, 27.54], [14.68, 20.13], [16.13, 20.68], [19.81, 22.15],
                [13.54, 14.36], [13.08, 15.71], [9.504, 12.44], [15.34, 14.26],
                [21.16, 23.04], [16.65, 21.38], [17.14, 16.4], [14.58, 21.53],
                [18.61, 20.25], [15.3, 25.27]],
                "csv_file_1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 2), list("rows", 13, 30))))
        )");
    }
}

void test_read_csv_3d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_read_csv_d_operation("test_read_csv_2loc3d_0", R"(
            file_read_csv_d(
                "test_20200713_2loc",
                true,
                10,
                "row",
                1,
                "test_1"
            )
        )", R"(
            annotate_d([[[22, 30], [17.99, 10.38], [20.57, 17.77],
                [19.69, 21.25], [11.42, 20.38], [20.29, 14.34]],
                [[12.46, 24.04], [16.02, 23.24], [15.78, 17.89],
                [19.17, 24.8], [15.85, 23.95], [13.73, 22.61]],
                [[13.54, 14.36], [13.08, 15.71], [9.504, 12.44],
                [15.34, 14.26], [21.16, 23.04], [16.65, 21.38]]],
                "test_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2),
                        list("pages", 0, 3), list("rows", 0, 6))))
        )");
    }
    else
    {
        test_read_csv_d_operation("test_read_csv_2loc3d_0", R"(
            file_read_csv_d(
                "test_20200713_2loc",
                true,
                10,
                "row",
                1,
                "test_1"
            )
        )", R"(
            annotate_d(
                [[[11.42, 20.38], [20.29, 14.34], [12.45, 15.7],
                [18.25, 19.98], [13.71, 20.83], [13, 21.82]],
                [[15.85, 23.95], [13.73, 22.61], [14.54, 27.54],
                [14.68, 20.13], [16.13, 20.68], [19.81, 22.15]],
                [[21.16, 23.04], [16.65, 21.38], [17.14, 16.4],
                [14.58, 21.53], [18.61, 20.25], [15.3, 25.27]]],
                "test_1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 2), list("rows", 4, 10),
                        list("pages", 0, 3))))
        )");
    }
}

void test_read_csv_3d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_read_csv_d_operation("test_read_csv_2loc3d_1", R"(
            file_read_csv_d(
                "test_20200713_2loc",
                true,
                10,
                "page"
            )
        )", R"(
            annotate_d([[[22, 30], [17.99, 10.38], [20.57, 17.77],
                [19.69, 21.25], [11.42, 20.38], [20.29, 14.34], [12.45, 15.7],
                [18.25, 19.98], [13.71, 20.83], [13, 21.82]], [[12.46, 24.04],
                [16.02, 23.24], [15.78, 17.89], [19.17, 24.8], [15.85, 23.95],
                [13.73, 22.61], [14.54, 27.54], [14.68, 20.13], [16.13, 20.68],
                [19.81, 22.15]]],
                "csv_file_2",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2),
                        list("pages", 0, 2), list("rows", 0, 10))))
        )");
    }
    else
    {
        test_read_csv_d_operation("test_read_csv_2loc3d_1", R"(
            file_read_csv_d(
                "test_20200713_2loc",
                true,
                10,
                "page"
            )
        )", R"(
            annotate_d(
                [[[13.54, 14.36], [13.08, 15.71], [9.504, 12.44], [15.34, 14.26],
                [21.16, 23.04], [16.65, 21.38], [17.14, 16.4], [14.58, 21.53],
                [18.61, 20.25], [15.3, 25.27]]],
                "csv_file_2",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 2), list("rows", 0, 10),
                        list("pages", 2, 3))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    std::string filename = "test_20200713_2loc";
    blaze::DynamicMatrix<double> in{{22, 30}, {17.99, 10.38}, {20.57, 17.77},
        {19.69, 21.25}, {11.42, 20.38}, {20.29, 14.34}, {12.45, 15.7},
        {18.25, 19.98}, {13.71, 20.83}, {13, 21.82}, {12.46, 24.04},
        {16.02, 23.24}, {15.78, 17.89}, {19.17, 24.8}, {15.85, 23.95},
        {13.73, 22.61}, {14.54, 27.54}, {14.68, 20.13}, {16.13, 20.68},
        {19.81, 22.15}, {13.54, 14.36}, {13.08, 15.71}, {9.504, 12.44},
        {15.34, 14.26}, {21.16, 23.04}, {16.65, 21.38}, {17.14, 16.4},
        {14.58, 21.53}, {18.61, 20.25}, {15.3, 25.27}};

    // write to file
    phylanx::execution_tree::primitive outfile =
        phylanx::execution_tree::primitives::create_file_write_csv(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                {filename}, phylanx::ir::node_data<double>(in)});

    auto f = outfile.eval();
    f.get();

    test_read_csv_2d_0();

    test_read_csv_3d_0();
    test_read_csv_3d_1();

    std::remove(filename.c_str());
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

