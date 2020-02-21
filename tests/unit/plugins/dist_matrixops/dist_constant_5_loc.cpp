// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

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

void test_constant_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_constant_5loc_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test5loc1d_0", R"(
            constant_d(42, list(13), 0, 5)
        )", R"(
            annotate_d([42.0, 42.0, 42.0], "constant_5_13",
                list("args",
                    list("locality", 0, 5),
                    list("tile", list("columns", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_constant_d_operation("test5loc1d_0", R"(
            constant_d(42, list(13), 1, 5)
        )", R"(
            annotate_d([42.0, 42.0, 42.0], "constant_5_13",
                list("args",
                    list("locality", 1, 5),
                    list("tile", list("columns", 3, 6))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_constant_d_operation("test5loc1d_0", R"(
            constant_d(42, list(13), 2, 5)
        )", R"(
            annotate_d([42.0, 42.0, 42.0], "constant_5_13",
                list("args",
                    list("locality", 2, 5),
                    list("tile", list("columns", 6, 9))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_constant_d_operation("test5loc1d_0", R"(
            constant_d(42, list(13), 3, 5)
        )", R"(
            annotate_d([42.0, 42.0], "constant_5_13",
                list("args",
                    list("locality", 3, 5),
                    list("tile", list("columns", 9, 11))))
        )");
    }
    else
    {
        test_constant_d_operation("test5loc1d_0", R"(
            constant_d(42, list(13), 4, 5)
        )", R"(
            annotate_d([42.0, 42.0], "constant_5_13",
                list("args",
                    list("locality", 4, 5),
                    list("tile", list("columns", 11, 13))))
        )");
    }
}





///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_constant_5loc_1d_0();


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

