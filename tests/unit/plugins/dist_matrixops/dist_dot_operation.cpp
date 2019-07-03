//   Copyright (c) 2017-2019 Hartmut Kaiser
//   Copyright (c) 2019 Bita Hasheminezhad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
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
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_dot_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(
        compile_and_run(name, code), compile_and_run(name, expected_str));
}

////////////////////////////////////////////////////////////////////////////////
void test_dot_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d_0",
            R"(
                dot_d(
                    annotate_d(
                        [1, 2, 3],
                        "test1d_0_1",
                        list("tile", list("columns", 0, 3))
                    ),
                    annotate_d(
                        [4, 5, 6],
                        "test1d_0_2",
                        list("tile", list("columns", 3, 6))
                    )
                )
            )",
            "91");
    }
    else
    {
        test_dot_operation("test1d_0",
            R"(
                dot_d(
                    annotate_d(
                        [4, 5, 6],
                        "test1d_0_1",
                        list("tile", list("columns", 3, 6))
                    ),
                    annotate_d(
                        [1, 2, 3],
                        "test1d_0_2",
                        list("tile", list("columns", 0, 3))
                    )
                )
            )",
            "91");
    }
}

void test_dot_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d_1",
            R"(
                dot_d(
                    annotate_d(
                        [1, 2, 3],
                        "test1d_1_1",
                        list("tile", list("columns", 0, 3))
                    ),
                    [1, 2, 3, 4, 5, 6]
                )
            )",
            "91");
    }
    else
    {
        test_dot_operation("test1d_1",
            R"(
                dot_d(
                    annotate_d(
                        [4, 5, 6],
                        "test1d_1_1",
                        list("tile", list("columns", 3, 6))
                    ),
                    [1, 2, 3, 4, 5, 6]
                )
            )",
            "91");
    }
}

void test_dot_1d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test1d_2",
            R"(
                dot_d(
                    [1, 2, 3, 4, 5, 6],
                    annotate_d(
                        [1, 2, 3],
                        "test1d_2_1",
                        list("tile", list("columns", 0, 3))
                    )
                )
            )",
            "91");
    }
    else
    {
        test_dot_operation("test1d_2",
            R"(
                dot_d(
                    [1, 2, 3, 4, 5, 6],
                    annotate_d(
                        [4, 5, 6],
                        "test1d_2_1",
                        list("tile", list("columns", 3, 6))
                    )
                )
            )",
            "91");
    }
}

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_dot_1d_0();
    test_dot_1d_1();
    test_dot_1d_2();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    return hpx::init(argc, argv, cfg);
}
