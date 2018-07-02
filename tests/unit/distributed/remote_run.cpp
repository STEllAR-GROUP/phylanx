// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #XXX: DESC_GOES_HERE
#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
bool is_locality_0 = false;

std::string code1 = "define(a, 1)";
std::string code2 = R"(block(
    define(fx, arg0, block(
        debug(arg0 + 4),
        debug(42)
    )),
    fx
))";

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(
    std::string const& code, std::uint32_t locality_id)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(
            hpx::naming::get_id_from_locality_id(locality_id));

    return phylanx::execution_tree::compile(code, snippets, env);
}

void test_remote_run()
{
    auto et = compile("debug(42)", 1);
    et();
}

void test_remote_run_chain()
{
    auto et1 = compile(code1, 1);
    auto r1 = et1();

    auto et2 = compile(code2, 1);
    et2(r1);
}

void test_remote_run_communicate()
{
    auto et1 = compile(code1, 1);
    auto r1 = et1();

    auto et2 = compile(code2, 0);
    et2(r1);
}

int hpx_main(int argc, char* argv[])
{
    is_locality_0 = hpx::naming::get_locality_id_from_id(hpx::find_here()) == 0;

    test_remote_run();
    test_remote_run_chain();
    test_remote_run_communicate();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    if (is_locality_0)
    {
        std::stringstream const& strm = hpx::get_consolestream();
        HPX_TEST_EQ(strm.str(), std::string("42\n5\n42\n5\n42\n"));
    }

    return hpx::util::report_errors();
}
