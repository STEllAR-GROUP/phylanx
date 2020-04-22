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

    HPX_TEST_EQ(result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_diag_4loc_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_diag_4loc_0", R"(
            diag_d(7, -4, 4, "mydiag0d_row", "row")
        )",
            R"(
            annotate_d([[1.0], [0.0], [0.0], [0.0]],
                "mydiag0d_row",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 1), list("rows", 0, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        
    }
    else if (hpx::get_locality_id() == 2)
    {
       
    }
    else if (hpx::get_locality_id() == 3)
    {
        
    }
}



///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_diag_4loc_0();


    hpx::finalize();
    return hpx::util::report_errors();
}
int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
