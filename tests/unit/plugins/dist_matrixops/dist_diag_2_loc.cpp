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

#include <string>
#include <utility>
#include <vector>

#include <iostream>
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

void test_diag_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    std::cout << result << "\n";

    HPX_TEST_EQ(result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_diag_2loc_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_diag_d_operation("test_diag_2loc_0", R"(
            diag_d([7], 3, 0, 2, "", "row")
        )",
            R"(
            annotate_d([[0, 0, 0, 7], [0, 0, 0, 0]],
                "diag_array_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 4), list("rows", 0, 2))))
        )");
    }
    else
    {
        test_diag_d_operation("test_diag_2loc_0", R"(
            diag_d([7], 3, 1, 2, "", "row")
        )",
            R"(
            annotate_d([[0, 0, 0, 0], [0, 0, 0, 0]],
                "diag_array_1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 4), list("rows", 2, 4))))
        )");
    }
}



///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_diag_2loc_0();


    hpx::finalize();
    return hpx::util::report_errors();
}
int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
