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
char const* const constant_code = "constant_d(42.0, list(13))";

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

void test_constant_d_operation(std::string const& name,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, constant_code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_constant_undefined_loc()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test_constant_undefined_loc_1d_0", R"(
            annotate_d([42.0, 42.0, 42.0, 42.0, 42.0], "full_array_1",
                list("args",
                    list("locality", 0, 3),
                    list("tile", list("columns", 0, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test_constant_undefined_loc_1d_0", R"(
            annotate_d([42.0, 42.0, 42.0, 42.0], "full_array_1",
                list("args",
                    list("locality", 1, 3),
                    list("tile", list("columns", 5, 9))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test_constant_undefined_loc_1d_0", R"(
            annotate_d([42.0, 42.0, 42.0, 42.0], "full_array_1",
                list("tile", list("columns", 9, 13)))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_constant_undefined_loc();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    hpx::init_params params;
    params.cfg = std::move(cfg);
    return hpx::init(argc, argv, params);
}

