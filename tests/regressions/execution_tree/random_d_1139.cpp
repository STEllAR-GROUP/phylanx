// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/testing.hpp>

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

    hpx::cout << "Locality: " << hpx::get_locality_id()
              << "  Starting execution for: " << name << hpx::endl;
    std::uint64_t t = hpx::util::high_resolution_clock::now();
    phylanx::execution_tree::primitive_argument_type a = compiled_code.run().arg_;
    t = hpx::util::high_resolution_clock::now() - t;

    std::cout << *(a.annotation()) << "\n\n\n";
    hpx::cout << "       " << (t / 1e6) << " ms.\n" << hpx::flush;
}

////////////////////////////////////////////////////////////////////////////////
void test__0()
{
    if (hpx::get_locality_id() == 0)
    {
        execution_time("perftest_0", R"(
            dot_d(
                constant_d(13., list(100, 100), 0, 2, "const_1"),
                constant_d(13., list(100, 100), 0, 2, "const_2")
            )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        execution_time("perftest_0", R"(
            dot_d(
                constant_d(13., list(100, 100), 1, 2, "const_1"),
                constant_d(13., list(100, 100), 1, 2, "const_2")
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
                random_d(list(100, 100), 0, 2, "rand_1"),
                random_d(list(100, 100), 0, 2, "rand_2")
            )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        execution_time("perftest_1", R"(
            dot_d(
                random_d(list(100, 100), 1, 2, "rand_1"),
                random_d(list(100, 100), 1, 2, "rand_2")
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

    return hpx::init(argc, argv, cfg);
}
