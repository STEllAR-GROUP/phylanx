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
    phylanx::execution_tree::primitive_argument_type cannon_result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);
    hpx::cout << cannon_result << " : " << comparison << hpx::endl;
    HPX_TEST_EQ(cannon_result, comparison);
}

void test_file_read_d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_file_read_d("testload_d_0", R"(
            define(x, random(make_list(1000, 1000))),
            file_write_csv("test_csv.csv", x),
            file_read_csv_d(
                "test_csv.csv",
                list("locality", 0, 2))
            )
        )",
            R"(
            annotate_d(
                slice_row(file_read_csv("test_csv.csv"),
                          make_list(0,500)),
                "test_csv.csv",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 3), list("rows", 0, 3))))
        )");
    }
    else
    {
        test_file_read_d("test2d2d_2", R"(
            cannon_product(
                annotate_d([[4], [5], [6]], "test2d2d_2_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 1, 2), list("rows", 3, 6)))),
                annotate_d([[4, 5, 6]], "test2d2d_2_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 3, 6), list("rows", 1, 2))))
            )
        )",
            R"(
            annotate_d([[32, 40, 48], [40, 50, 60], [48, 60, 72]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 3, 6), list("rows", 3, 6))))
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

int main(int argc, char* argv[])
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};

    //test_file_io(phylanx::ir::node_data<double>(42.0));

    blaze::DynamicVector<double> ev = gen.generate(1007UL);
    //test_file_io(phylanx::ir::node_data<double>(std::move(ev)));

    std::vector<double> v(1007);
    std::generate(v.begin(), v.end(), std::rand);
    //test_file_io(phylanx::ir::node_data<double>(std::move(v)));

    blaze::Rand<blaze::DynamicMatrix<double>> gen2{};

    blaze::DynamicMatrix<double> m = gen2.generate(101UL, 101UL);
    //test_file_io(phylanx::ir::node_data<double>(std::move(m)));

    return hpx::util::report_errors();
}
