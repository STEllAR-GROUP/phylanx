//   Copyright (c) 2017-2019 Hartmut Kaiser
//   Copyright (c) 2019 Bita Hasheminezhad
//   Copyright (c) 2019 Maxwell Reeser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/hpx_main.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/testing.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
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

///////////////////////////////////////////////////////////////////////////////
void time_mat_mul(std::string const& name, std::string const& code)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& compiled_code =
        phylanx::execution_tree::compile(name, code, snippets, env);
    hpx::cout << "Locality: " << hpx::get_locality_id()
              << " Starting execution for: " << name << hpx::endl;
    std::chrono::high_resolution_clock::time_point t1 =
        std::chrono::high_resolution_clock::now();
    phylanx::execution_tree::primitive_argument_type val =
        compiled_code.run().arg_;
    std::chrono::high_resolution_clock::time_point t2 =
        std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_span = t2 - t1;
    hpx::cout << "Locality: " << hpx::get_locality_id() << " " << name
              << " elapsed time: " << time_span.count() << hpx::endl;
    hpx::cout << hpx::flush;
}

////////////////////////////////////////////////////////////////////////////////
void test_dot_d_100()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_100", R"(
            dot_d(
                annotate_d(
                    random(make_list(100,100)), "perftest_matmul_dot_d_0_1",
                    list("tile", list("columns", 0, 100), list("rows", 0, 100))
                ),
                annotate_d(
                    random(make_list(100,100)), "perftest_matmul_dot_d_0_2",
                    list("tile", list("columns", 0, 100), list("rows", 0, 100))
                )
            )
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_100", R"(
            dot_d(
                annotate_d(
                    random(make_list(100,100)), "perftest_matmul_dot_d_0_1",
                    list("tile", list("columns", 100, 200), list("rows", 0, 100))
                ),
                annotate_d(
                    random(make_list(100,100)), "perftest_matmul_dot_d_0_2",
                    list("tile", list("columns", 100, 200), list("rows", 0, 100))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_100", R"(
            dot_d(
                annotate_d(
                    random(make_list(100,100)), "perftest_matmul_dot_d_0_1",
                    list("tile", list("columns", 0, 100), list("rows", 100, 200))
                ),
                annotate_d(
                    random(make_list(100,100)), "perftest_matmul_dot_d_0_2",
                    list("tile", list("columns", 0, 100), list("rows", 100, 200))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_100", R"(
            dot_d(
                annotate_d(
                    random(make_list(100,100)), "perftest_matmul_dot_d_0_1",
                    list("tile", list("columns", 100, 200), list("rows", 100, 200))
                ),
                annotate_d(
                    random(make_list(100,100)), "perftest_matmul_dot_d_0_2",
                    list("tile", list("columns", 100, 200), list("rows", 100, 200))
                )
            )
        )");
    }
}

void test_dot_d_200()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_200", R"(
            dot_d(
                annotate_d(
                    random(make_list(200,200)), "perftest_matmul_dot_d_2_1",
                    list("tile", list("columns", 0, 200), list("rows", 0, 200))
                ),
                annotate_d(
                    random(make_list(200,200)), "perftest_matmul_dot_d_2_2",
                    list("tile", list("columns", 0, 200), list("rows", 0, 200))
                )
            )
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_200", R"(
            dot_d(
                annotate_d(
                    random(make_list(200,200)), "perftest_matmul_dot_d_2_1",
                    list("tile", list("columns", 200, 400), list("rows", 0, 200))
                ),
                annotate_d(
                    random(make_list(200,200)), "perftest_matmul_dot_d_2_2",
                    list("tile", list("columns", 200, 400), list("rows", 0, 200))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_200", R"(
            dot_d(
                annotate_d(
                    random(make_list(200,200)), "perftest_matmul_dot_d_2_1",
                    list("tile", list("columns", 0, 200), list("rows", 200, 400))
                ),
                annotate_d(
                    random(make_list(200,200)), "perftest_matmul_dot_d_2_2",
                    list("tile", list("columns", 0, 200), list("rows", 200, 400))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_200", R"(
            dot_d(
                annotate_d(
                    random(make_list(200,200)), "perftest_matmul_dot_d_2_1",
                    list("tile", list("columns", 200, 400), list("rows", 200, 400))
                ),
                annotate_d(
                    random(make_list(200,200)), "perftest_matmul_dot_d_2_2",
                    list("tile", list("columns", 200, 400), list("rows", 200, 400))
                )
            )
        )");
    }
}

void test_dot_d_210()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_210", R"(
            dot_d(
                annotate_d(
                    random(make_list(210,210)), "perftest_matmul_dot_d_9_1",
                    list("tile", list("columns", 0, 210), list("rows", 0, 210))
                ),
                annotate_d(
                    random(make_list(210,210)), "perftest_matmul_dot_d_9_2",
                    list("tile", list("columns", 0, 210), list("rows", 0, 210))
                )
            )
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_210", R"(
            dot_d(
                annotate_d(
                    random(make_list(210,210)), "perftest_matmul_dot_d_9_1",
                    list("tile", list("columns", 210, 420), list("rows", 0, 210))
                ),
                annotate_d(
                    random(make_list(210,210)), "perftest_matmul_dot_d_9_2",
                    list("tile", list("columns", 210, 420), list("rows", 0, 210))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_210", R"(
            dot_d(
                annotate_d(
                    random(make_list(210,210)), "perftest_matmul_dot_d_9_1",
                    list("tile", list("columns", 0, 210), list("rows", 210, 420))
                ),
                annotate_d(
                    random(make_list(210,210)), "perftest_matmul_dot_d_9_2",
                    list("tile", list("columns", 0, 210), list("rows", 210, 420))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_210", R"(
            dot_d(
                annotate_d(
                    random(make_list(210,210)), "perftest_matmul_dot_d_9_1",
                    list("tile", list("columns", 210, 420), list("rows", 210, 420))
                ),
                annotate_d(
                    random(make_list(210,210)), "perftest_matmul_dot_d_9_2",
                    list("tile", list("columns", 210, 420), list("rows", 210, 420))
                )
            )
        )");
    }
}

void test_dot_d_225()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_225", R"(
            dot_d(
                annotate_d(
                    random(make_list(225,225)), "perftest_matmul_dot_d_8_1",
                    list("tile", list("columns", 0, 225), list("rows", 0, 225))
                ),
                annotate_d(
                    random(make_list(225,225)), "perftest_matmul_dot_d_8_2",
                    list("tile", list("columns", 0, 225), list("rows", 0, 225))
                )
            )
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_225", R"(
            dot_d(
                annotate_d(
                    random(make_list(225,225)), "perftest_matmul_dot_d_8_1",
                    list("tile", list("columns", 225, 450), list("rows", 0, 225))
                ),
                annotate_d(
                    random(make_list(225,225)), "perftest_matmul_dot_d_8_2",
                    list("tile", list("columns", 225, 450), list("rows", 0, 225))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_225", R"(
            dot_d(
                annotate_d(
                    random(make_list(225,225)), "perftest_matmul_dot_d_8_1",
                    list("tile", list("columns", 0, 225), list("rows", 225, 450))
                ),
                annotate_d(
                    random(make_list(225,225)), "perftest_matmul_dot_d_8_2",
                    list("tile", list("columns", 0, 225), list("rows", 225, 450))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_225", R"(
            dot_d(
                annotate_d(
                    random(make_list(225,225)), "perftest_matmul_dot_d_8_1",
                    list("tile", list("columns", 225, 450), list("rows", 225, 450))
                ),
                annotate_d(
                    random(make_list(225,225)), "perftest_matmul_dot_d_8_2",
                    list("tile", list("columns", 225, 450), list("rows", 225, 450))
                )
            )
        )");
    }
}

void test_dot_d_250()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_250", R"(
            dot_d(
                annotate_d(
                    random(make_list(250,250)), "perftest_matmul_dot_d_7_1",
                    list("tile", list("columns", 0, 250), list("rows", 0, 250))
                ),
                annotate_d(
                    random(make_list(250,250)), "perftest_matmul_dot_d_7_2",
                    list("tile", list("columns", 0, 250), list("rows", 0, 250))
                )
            )
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_250", R"(
            dot_d(
                annotate_d(
                    random(make_list(250,250)), "perftest_matmul_dot_d_7_1",
                    list("tile", list("columns", 250, 500), list("rows", 0, 250))
                ),
                annotate_d(
                    random(make_list(250,250)), "perftest_matmul_dot_d_7_2",
                    list("tile", list("columns", 250, 500), list("rows", 0, 250))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_250", R"(
            dot_d(
                annotate_d(
                    random(make_list(250,250)), "perftest_matmul_dot_d_7_1",
                    list("tile", list("columns", 0, 250), list("rows", 250, 500))
                ),
                annotate_d(
                    random(make_list(250,250)), "perftest_matmul_dot_d_7_2",
                    list("tile", list("columns", 0, 250), list("rows", 250, 500))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_250", R"(
            dot_d(
                annotate_d(
                    random(make_list(250,250)), "perftest_matmul_dot_d_7_1",
                    list("tile", list("columns", 250, 500), list("rows", 250, 500))
                ),
                annotate_d(
                    random(make_list(250,250)), "perftest_matmul_dot_d_7_2",
                    list("tile", list("columns", 250, 500), list("rows", 250, 500))
                )
            )
        )");
    }
}

void test_dot_d_300()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_300", R"(
            dot_d(
                annotate_d(
                    random(make_list(300,300)), "perftest_matmul_dot_d_6_1",
                    list("tile", list("columns", 0, 300), list("rows", 0, 300))
                ),
                annotate_d(
                    random(make_list(300,300)), "perftest_matmul_dot_d_6_2",
                    list("tile", list("columns", 0, 300), list("rows", 0, 300))
                )
            )
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_300", R"(
            dot_d(
                annotate_d(
                    random(make_list(300,300)), "perftest_matmul_dot_d_6_1",
                    list("tile", list("columns", 300, 600), list("rows", 0, 300))
                ),
                annotate_d(
                    random(make_list(300,300)), "perftest_matmul_dot_d_6_2",
                    list("tile", list("columns", 300, 600), list("rows", 0, 300))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_300", R"(
            dot_d(
                annotate_d(
                    random(make_list(300,300)), "perftest_matmul_dot_d_6_1",
                    list("tile", list("columns", 0, 300), list("rows", 300, 600))
                ),
                annotate_d(
                    random(make_list(300,300)), "perftest_matmul_dot_d_6_2",
                    list("tile", list("columns", 0, 300), list("rows", 300, 600))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_300", R"(
            dot_d(
                annotate_d(
                    random(make_list(300,300)), "perftest_matmul_dot_d_6_1",
                    list("tile", list("columns", 300, 600), list("rows", 300, 600))
                ),
                annotate_d(
                    random(make_list(300,300)), "perftest_matmul_dot_d_6_2",
                    list("tile", list("columns", 300, 600), list("rows", 300, 600))
                )
            )
        )");
    }
}

void test_dot_d_400()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_400", R"(
            dot_d(
                annotate_d(
                    random(make_list(400,400)), "perftest_matmul_dot_d_3_1",
                    list("tile", list("columns", 0, 400), list("rows", 0, 400))
                ),
                annotate_d(
                    random(make_list(400,400)), "perftest_matmul_dot_d_3_2",
                    list("tile", list("columns", 0, 400), list("rows", 0, 400))
                )
            )
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_400", R"(
            dot_d(
                annotate_d(
                    random(make_list(400,400)), "perftest_matmul_dot_d_3_1",
                    list("tile", list("columns", 400, 800), list("rows", 0, 400))
                ),
                annotate_d(
                    random(make_list(400,400)), "perftest_matmul_dot_d_3_2",
                    list("tile", list("columns", 400, 800), list("rows", 0, 400))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_400", R"(
            dot_d(
                annotate_d(
                    random(make_list(400,400)), "perftest_matmul_dot_d_3_1",
                    list("tile", list("columns", 0, 400), list("rows", 400, 800))
                ),
                annotate_d(
                    random(make_list(400,400)), "perftest_matmul_dot_d_3_2",
                    list("tile", list("columns", 0, 400), list("rows", 400, 800))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_400", R"(
            dot_d(
                annotate_d(
                    random(make_list(400,400)), "perftest_matmul_dot_d_3_1",
                    list("tile", list("columns", 400, 800), list("rows", 400, 800))
                ),
                annotate_d(
                    random(make_list(400,400)), "perftest_matmul_dot_d_3_2",
                    list("tile", list("columns", 400, 800), list("rows", 400, 800))
                )
            )
        )");
    }
}

void test_dot_d_600()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_600", R"(
            dot_d(
                annotate_d(
                    random(make_list(600,600)), "perftest_matmul_dot_d_4_1",
                    list("tile", list("columns", 0, 600), list("rows", 0, 600))
                ),
                annotate_d(
                    random(make_list(600,600)), "perftest_matmul_dot_d_4_2",
                    list("tile", list("columns", 0, 600), list("rows", 0, 600))
                )
            )
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_600", R"(
            dot_d(
                annotate_d(
                    random(make_list(600,600)), "perftest_matmul_dot_d_4_1",
                    list("tile", list("columns", 600, 1200), list("rows", 0, 600))
                ),
                annotate_d(
                    random(make_list(600,600)), "perftest_matmul_dot_d_4_2",
                    list("tile", list("columns", 600, 1200), list("rows", 0, 600))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_600", R"(
            dot_d(
                annotate_d(
                    random(make_list(600,600)), "perftest_matmul_dot_d_4_1",
                    list("tile", list("columns", 0, 600), list("rows", 600, 1200))
                ),
                annotate_d(
                    random(make_list(600,600)), "perftest_matmul_dot_d_4_2",
                    list("tile", list("columns", 0, 600), list("rows", 600, 1200))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_600", R"(
            dot_d(
                annotate_d(
                    random(make_list(600,600)), "perftest_matmul_dot_d_4_1",
                    list("tile", list("columns", 600, 1200), list("rows", 600, 1200))
                ),
                annotate_d(
                    random(make_list(600,600)), "perftest_matmul_dot_d_4_2",
                    list("tile", list("columns", 600, 1200), list("rows", 600, 1200))
                )
            )
        )");
    }
}

void test_dot_d_800()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_800", R"(
            dot_d(
                annotate_d(
                    random(make_list(800,800)), "perftest_matmul_dot_d_5_1",
                    list("tile", list("columns", 0, 800), list("rows", 0, 800))
                ),
                annotate_d(
                    random(make_list(800,800)), "perftest_matmul_dot_d_5_2",
                    list("tile", list("columns", 0, 800), list("rows", 0, 800))
                )
            )
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_800", R"(
            dot_d(
                annotate_d(
                    random(make_list(800,800)), "perftest_matmul_dot_d_5_1",
                    list("tile", list("columns", 800, 1600), list("rows", 0, 800))
                ),
                annotate_d(
                    random(make_list(800,800)), "perftest_matmul_dot_d_5_2",
                    list("tile", list("columns", 800, 1600), list("rows", 0, 800))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_800", R"(
            dot_d(
                annotate_d(
                    random(make_list(800,800)), "perftest_matmul_dot_d_5_1",
                    list("tile", list("columns", 0, 800), list("rows", 800, 1600))
                ),
                annotate_d(
                    random(make_list(800,800)), "perftest_matmul_dot_d_5_2",
                    list("tile", list("columns", 0, 800), list("rows", 800, 1600))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_800", R"(
            dot_d(
                annotate_d(
                    random(make_list(800,800)), "perftest_matmul_dot_d_5_1",
                    list("tile", list("columns", 800, 1600), list("rows", 800, 1600))
                ),
                annotate_d(
                    random(make_list(800,800)), "perftest_matmul_dot_d_5_2",
                    list("tile", list("columns", 800, 1600), list("rows", 800, 1600))
                )
            )
        )");
    }
}

void test_dot_d_1000()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test_dot_d_1000", R"(
            dot_d(
                annotate_d(
                    random(make_list(1000,1000)), "perftest_matmul_dot_d_1_1",
                    list("tile", list("columns", 0, 1000), list("rows", 0, 1000))
                ),
                annotate_d(
                    random(make_list(1000,1000)), "perftest_matmul_dot_d_1_2",
                    list("tile", list("columns", 0, 1000), list("rows", 0, 1000))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test_dot_d_1000", R"(
            dot_d(
                annotate_d(
                    random(make_list(1000,1000)), "perftest_matmul_dot_d_1_1",
                    list("tile", list("columns", 1000, 2000), list("rows", 0, 1000))
                ),
                annotate_d(
                    random(make_list(1000,1000)), "perftest_matmul_dot_d_1_2",
                    list("tile", list("columns", 1000, 2000), list("rows", 0, 1000))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test_dot_d_1000", R"(
            dot_d(
                annotate_d(
                    random(make_list(1000,1000)), "perftest_matmul_dot_d_1_1",
                    list("tile", list("columns", 0, 1000), list("rows", 1000, 2000))
                ),
                annotate_d(
                    random(make_list(1000,1000)), "perftest_matmul_dot_d_1_2",
                    list("tile", list("columns", 0, 1000), list("rows", 1000, 2000))
                )
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        time_mat_mul("test_dot_d_1000", R"(
            dot_d(
                annotate_d(
                    random(make_list(1000,1000)), "perftest_matmul_dot_d_1_1",
                    list("tile", list("columns", 1000, 2000), list("rows", 1000, 2000))
                ),
                annotate_d(
                    random(make_list(1000,1000)), "perftest_matmul_dot_d_1_2",
                    list("tile", list("columns", 1000, 2000), list("rows", 1000, 2000))
                )
            )
        )");
    }
}

void test_cannon_0()
{
    if (hpx::get_locality_id() == 0)
    {
        time_mat_mul("test2d2d_2", R"(
            cannon_product(
                annotate_d([[1], [2], [3]], "test2d2d_2_1",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 1), list("rows", 0, 3)))),
                annotate_d([[1, 2, 3]], "test2d2d_2_2",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 3), list("rows", 0, 1))))
            )
        )");
        //             ,
        //    R"(
        //    annotate_d([[2, 4, 6], [4, 8, 12], [6, 12, 18]],
        //        "test2d2d_2_1/1",
        //        list("args",
        //            list("locality", 0, 4),
        //            list("tile", list("columns", 0, 3), list("rows", 0, 3))))
        //)");
    }
    else if (hpx::get_locality_id() == 1)
    {
        time_mat_mul("test2d2d_2", R"(
            cannon_product(
                annotate_d([[1], [2], [3]], "test2d2d_2_1",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 1, 2), list("rows", 0, 3)))),
                annotate_d([[4, 5, 6]], "test2d2d_2_2",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 3, 6), list("rows", 0, 1))))
            )
        )");
        //,
        //    R"(
        //    annotate_d([[8, 10, 12], [16, 20, 24], [24, 30, 36]],
        //        "test2d2d_2_1/1",
        //        list("args",
        //            list("locality", 1, 4),
        //            list("tile", list("columns", 3, 6), list("rows", 0, 3))))
        //)");
    }
    else if (hpx::get_locality_id() == 2)
    {
        time_mat_mul("test2d2d_2", R"(
            cannon_product(
                annotate_d([[4], [5], [6]], "test2d2d_2_1",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 1), list("rows", 3, 6)))),
                annotate_d([[1, 2, 3]], "test2d2d_2_2",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 3), list("rows", 1, 2))))
            )
        )");
        //,
        //    R"(
        //    annotate_d([[8, 16, 24], [10, 20, 30], [12, 24, 36]],
        //        "test2d2d_2_1/1",
        //        list("args",
        //            list("locality", 2, 4),
        //            list("tile", list("columns", 0, 3), list("rows", 3, 6))))
        //)");
    }
    else
    {
        time_mat_mul("test2d2d_2", R"(
            cannon_product(
                annotate_d([[4], [5], [6]], "test_2_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 1, 2), list("rows", 3, 6)))),
                annotate_d([[4, 5, 6]], "test2d2d_2_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 3, 6), list("rows", 1, 2))))
            )
        )");
        /*,
            R"(
            annotate_d([[32, 40, 48], [40, 50, 60], [48, 60, 72]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 3, 6), list("rows", 3, 6))))
        )");*/
    }
}

int hpx_main(int argc, char* argv[])
{
   /*
    test_dot_d_100();
    test_dot_d_200();
    */
    test_dot_d_210();
    test_dot_d_225();
   /*
   test_dot_d_250();
    test_dot_d_300();
    test_dot_d_400();
    test_dot_d_600();
    test_dot_d_800();
    test_dot_d_1000();
    */
    //test_cannon_0();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {};

    return hpx::init(argc, argv, cfg);
}
