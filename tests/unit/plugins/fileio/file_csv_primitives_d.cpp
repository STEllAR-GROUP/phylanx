//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

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
void test_file_read_d(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type dist_read =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);
    //hpx::cout << dist_read << " : " << comparison << hpx::endl;
    HPX_TEST_EQ(dist_read, comparison);
}

void test_file_read_d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        compile_and_run("test_make_csv", R"(
            file_write_csv("test_csv.csv", random(make_list(40,40)))
        )");
        test_file_read_d("testload_d_0", R"(
            file_read_csv_d("test_csv.csv", list("locality", 0, 2))
        )",
            R"(
            annotate_d(
                slice_row(file_read_csv("test_csv.csv"), make_list(0,20)),
                "test_csv.csvfile_read_csv_d",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 40), list("rows", 0, 40)))
            )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_file_read_d("testload_d_0", R"(
            file_read_csv_d("test_csv.csv", list("locality", 1, 2))
        )",
            R"(
            annotate_d(
                slice_row(file_read_csv("test_csv.csv"), make_list(20, 40)),
                "test_csv.csvfile_read_csv_d",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 40), list("rows", 0, 40))))
        )");
    }
}

//void test_file_io_lit(phylanx::ir::node_data<double> const& in)
//{
//    std::string filename = std::tmpnam(nullptr);
//
//    // write to file
//    if (hpx::get_locality_id() == 0)
//    {
//        phylanx::execution_tree::primitive litval =
//            phylanx::execution_tree::primitives::create_variable(
//                hpx::find_here(), in);
//
//        phylanx::execution_tree::primitive outfile =
//            phylanx::execution_tree::primitives::create_file_write_csv(
//                hpx::find_here(),
//                phylanx::execution_tree::primitive_arguments_type{
//                    {filename}, litval});
//
//        auto f = outfile.eval();
//        f.get();
//    }
//
//    // read back the file
//    hpx::future<phylanx::execution_tree::primitive_argument_type> f;
//    {
//        phylanx::ir::range loc_info;
//        if (hpx::get_locality_id() == 0)
//        {
//            loc_info = phylanx::ir::range{"locality", 0, 2};
//        }
//        else if (hpx::get_locality_id() == 1)
//        {
//            loc_info = phylanx::ir::range{"locality", 1, 2};
//        }
//        phylanx::execution_tree::primitive_argument_type p{loc_info};
//        phylanx::execution_tree::primitive infile =
//            phylanx::execution_tree::primitives::create_file_read_csv_d(
//                hpx::find_here(),
//                phylanx::execution_tree::primitive_arguments_type{
//                    {filename}, {p}});
//
//        f = infile.eval();
//    }
//    // Have to check that this is sliced correctly for comparison
//    HPX_TEST(in == phylanx::execution_tree::extract_numeric_value(f.get()));
//
//    std::remove(filename.c_str());
//}
//
//void test_file_io_primitive(phylanx::ir::node_data<double> const& in)
//{
//    std::string filename = std::tmpnam(nullptr);
//
//    // write to file
//    {
//        phylanx::execution_tree::primitive outfile =
//            phylanx::execution_tree::primitives::create_file_write_csv(
//                hpx::find_here(),
//                phylanx::execution_tree::primitive_arguments_type{
//                    {filename}, in});
//
//        auto f = outfile.eval();
//        f.get();
//    }
//
//    // read back the file
//    hpx::future<phylanx::execution_tree::primitive_argument_type> f;
//    {
//        phylanx::ir::range loc_info;
//        if (hpx::get_locality_id() == 0)
//        {
//            loc_info = phylanx::ir::range{"locality", 0, 2};
//        }
//        else if (hpx::get_locality_id() == 1)
//        {
//            loc_info = phylanx::ir::range{"locality", 1, 2};
//        }
//        phylanx::execution_tree::primitive_argument_type p{loc_info};
//        phylanx::execution_tree::primitive infile =
//            phylanx::execution_tree::primitives::create_file_read_csv_d(
//                hpx::find_here(),
//                phylanx::execution_tree::primitive_arguments_type{
//                    {filename}, {p}});
//
//        f = infile.eval();
//    }
//
//    HPX_TEST(in == phylanx::execution_tree::extract_numeric_value(f.get()));
//
//    std::remove(filename.c_str());
//}
//
//void test_file_io(phylanx::ir::node_data<double> const& in)
//{
//    test_file_io_lit(in);
//    test_file_io_primitive(in);
//}

int hpx_main(int argc, char* argv[])
{
    test_file_read_d_0();
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    hpx::cout << "Hello" << hpx::endl;

    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
