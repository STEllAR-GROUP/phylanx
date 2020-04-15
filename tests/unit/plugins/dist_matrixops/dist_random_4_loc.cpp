// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
char const* const random_code = "random_d(list(5, 6))";

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

void test_random_d_operation(std::string const& name,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, random_code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    // comparing annotations
    HPX_TEST_EQ(*(result.annotation()),*(comparison.annotation()));
}

///////////////////////////////////////////////////////////////////////////////
void test_random_undefined_loc()
{
    if (hpx::get_locality_id() == 0)
    {
        test_random_d_operation("test_random_undefined_loc_2d_0", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0]], "random_array_1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 3), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_random_d_operation("test_random_undefined_loc_2d_0", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0],
                        [42.0, 42.0, 42.0]], "random_array_1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 3, 6), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_random_d_operation("test_random_undefined_loc_2d_0", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                "random_array_1",
                list("tile", list("columns", 0, 3), list("rows", 3, 5)))
        )");
    }
    else
    {
        test_random_d_operation("test_random_undefined_loc_2d_0", R"(
            annotate_d([[42.0, 42.0, 42.0], [42.0, 42.0, 42.0]],
                "random_array_1",
                list("tile", list("columns", 3, 6), list("rows", 3, 5)))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    // only annotations are compared
    test_random_undefined_loc();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}

