// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
char const* const conv1d_d_code1= R"(
    conv1d_d(
        constant_d(1, list(8, 6, 3)),
        constant(-1, list(4, 3, 5)),
        "valid",
        1,
        1,
        "data"
    ))";
char const* const res_code1 = R"(constant_d(-12, list(8, 3, 5)))";

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

void test_conv1d_d_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    std::cout << result << std::endl;

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_conv1d_d_2d()
{
    if (hpx::get_locality_id() == 0)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d_2d_0", conv1d_d_code1, res_code1);
    }
    else
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d_2d_0", conv1d_d_code1, res_code1);
    }
}


///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    // only annotations are compared
    test_conv1d_d_2d();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
