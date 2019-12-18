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
