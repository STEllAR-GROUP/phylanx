// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #XXX: DESC_GOES_HERE
#include <phylanx/execution_tree/primitives/generic_function.hpp>
#include <phylanx/phylanx.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    hpx::future<primitive_argument_type> locality_id(
        std::vector<primitive_argument_type> const&,
        std::vector<primitive_argument_type> const&,
        std::string const&, std::string const&);
}}}

HPX_PLAIN_ACTION(
    phylanx::execution_tree::primitives::locality_id, locality_id_action);

namespace phylanx { namespace execution_tree { namespace primitives
{
    match_pattern_type const locality_id_match_data =
    {
        hpx::util::make_tuple(
            "locality_id", std::vector<std::string>{"locality_id()"},
            &create_generic_function<locality_id_action>,
            &create_primitive<generic_function<locality_id_action>>)
    };

    hpx::future<primitive_argument_type> locality_id(
        std::vector<primitive_argument_type> const&,
        std::vector<primitive_argument_type> const&,
        std::string const&, std::string const&)
    {
        std::int64_t locality_ =
            hpx::naming::get_locality_id_from_id(hpx::find_here());
        return hpx::make_ready_future(primitive_argument_type(locality_));
    }
}}}

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

void test_remote_run_on_0()
{
    auto et = compile("debug(locality_id())", 0);
    et();
}

void test_remote_run_on_1()
{
    auto et = compile("debug(locality_id())", 1);
    et();
}

void test_remote_run_chain()
{
    auto et1 = compile(code1, 1);
    auto r1 = et1();

    auto et2 = compile(code2, 1);
    et2(r1);
}

void test_remote_run_communicate_0_to_1()
{
    auto et1 = compile(code1, 0);
    auto r1 = et1();

    auto et2 = compile(code2, 1);
    et2(r1);
}

void test_remote_run_communicate_1_to_0()
{
    auto et1 = compile(code1, 1);
    auto r1 = et1();

    auto et2 = compile(code2, 0);
    et2(r1);
}

int hpx_main(int argc, char* argv[])
{
    HPX_TEST(hpx::get_num_localities(hpx::launch::sync) >= 2);

    is_locality_0 = hpx::naming::get_locality_id_from_id(hpx::find_here()) == 0;

    test_remote_run_on_0();
    test_remote_run_on_1();
    test_remote_run_chain();
    test_remote_run_communicate_0_to_1();
    test_remote_run_communicate_1_to_0();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    phylanx::execution_tree::register_pattern("locality_id_action",
        phylanx::execution_tree::primitives::locality_id_match_data);

    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    if (is_locality_0)
    {
        std::stringstream const& strm = hpx::get_consolestream();
        HPX_TEST_EQ(strm.str(), std::string("0\n1\n5\n42\n5\n42\n5\n42\n"));
    }

    return hpx::util::report_errors();
}
