// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/modules/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include<iostream>

///////////////////////////////////////////////////////////////////////////////
void execution_time(std::string const& name, std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& compiled_code =
        phylanx::execution_tree::compile(name, codestr, snippets, env);

    std::cout << "Locality: " << hpx::get_locality_id()
              << "  Starting execution for: " << name << std::endl;
    std::uint64_t t = hpx::chrono::high_resolution_clock::now();
    phylanx::execution_tree::primitive_argument_type a = compiled_code.run().arg_;
    t = hpx::chrono::high_resolution_clock::now() - t;

    std::cout << "       " << (t / 1e6) << " ms.\n" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void test__0()
{
    if (hpx::get_locality_id() == 0)
    {
        execution_time("perftest_0", R"(
            dot_d(
                constant_d(13., list(10, 10), 0, 2, "const_1"),
                constant_d(13., list(10, 10), 0, 2, "const_2")
            )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        execution_time("perftest_0", R"(
            dot_d(
                constant_d(13., list(10, 10), 1, 2, "const_1"),
                constant_d(13., list(10, 10), 1, 2, "const_2")
            )
        )");
    }
}

void test__1()
{
    if (hpx::get_locality_id() == 0)
    {
        execution_time("perftest_1", R"(
            dot_d(
                random_d(list(10, 10), 0, 2, "rand_1"),
                random_d(list(10, 10), 0, 2, "rand_2")
            )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        execution_time("perftest_1", R"(
            dot_d(
                random_d(list(10, 10), 1, 2, "rand_1"),
                random_d(list(10, 10), 1, 2, "rand_2")
            )
        )");
    }
}

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test__0();
    test__1();

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
