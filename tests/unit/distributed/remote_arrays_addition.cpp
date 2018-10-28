// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/execution_tree/primitives/generic_function.hpp>
#include <phylanx/phylanx.hpp>
#include <phylanx/plugins/plugin_factory.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    hpx::future<primitive_argument_type> locality_id(
        phylanx::execution_tree::primitive_arguments_type const&,
        phylanx::execution_tree::primitive_arguments_type const&,
        std::string const&, std::string const&);
}}}

HPX_PLAIN_ACTION(
    phylanx::execution_tree::primitives::locality_id, locality_id_action);

namespace phylanx { namespace execution_tree { namespace primitives {
    match_pattern_type const locality_id_match_data = {hpx::util::make_tuple(
        "locality_id", std::vector<std::string>{"locality_id()"},
        &create_generic_function<locality_id_action>,
        &create_primitive<generic_function<locality_id_action>>,
        "\n"
        "Args:\n"
        "\n"
        "Returns:\n"
        "\n"
        "The locality of the currently executing code")};

    hpx::future<primitive_argument_type> locality_id(
        phylanx::execution_tree::primitive_arguments_type const&,
        phylanx::execution_tree::primitive_arguments_type const&,
        std::string const&, std::string const&)
    {
        std::int64_t locality_ =
            hpx::naming::get_locality_id_from_id(hpx::find_here());
        return hpx::make_ready_future(primitive_argument_type(locality_));
    }
}}}

///////////////////////////////////////////////////////////////////////////////
bool is_locality_0 = false;

std::string init_arrays = R"(block(
    define(arrays_init,
        block(
            define(a, constant(41.0, list(2, 4))),
            define(rowsA, shape(a, 0)),
            define(columnsA, shape(a, 1)),
            define(a1, slice(a, list(0, rowsA), list(0, columnsA / 2))),
            define(a2, slice(a, list(0, rowsA), list(columnsA / 2, columnsA))),
            // cout(a),
            // cout(a1),
            // cout(a2),

            define(b, constant(1.0, list(2, 4))),
            define(rowsB, shape(b, 0)),
            define(columnsB, shape(b, 1)),
            define(b1, slice(b, list(0, rowsB), list(0, columnsB / 2))),
            define(b2, slice(b, list(0, rowsB), list(columnsB / 2, columnsB))),

            // cout(b),
            // cout(b1),
            // cout(b2),
            list(a1, a2, b1, b2)
        )
    ),
    arrays_init
))";

std::string code1 = R"(block(
    define(code1, arg0, arg1,
        block(
            define(c1, __add(arg0, arg1)),
            debug(c1)
        )
    ),
    code1
))";

std::string code2 = R"(block(
    define(code2, arg0, arg1,
        block(
            define(c2, __add(arg0, arg1)),
            debug(c2)
        )
    ),
    code2
))";

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(
    std::string const& codestr, std::uint32_t locality_id)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(
            hpx::naming::get_id_from_locality_id(locality_id));

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run();
}

phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run();
}

void test_remote_arrays_chain(std::uint32_t here, std::uint32_t there)
{
    auto temp = compile_and_run(init_arrays);
    auto arrays_list = temp();
    auto arrays_list_value =
        phylanx::execution_tree::extract_list_value(arrays_list);
    HPX_TEST_EQ(arrays_list_value.size(), 4);
    auto it = arrays_list_value.begin();
    auto a1 = *it++;
    auto a2 = *it++;
    auto b1 = *it++;
    auto b2 = *it++;

    auto et1 = compile(code1, here);
    auto r1 = et1(a1, b1);

    auto et2 = compile(code2, there);
    et2(a2, b2);
}

void test_remote_arrays_on(std::uint32_t there)
{
    auto et = compile("debug(locality_id())", there);
    et();
}

int hpx_main(int argc, char* argv[])
{
    HPX_TEST(hpx::get_num_localities(hpx::launch::sync) >= 2);

    is_locality_0 = hpx::naming::get_locality_id_from_id(hpx::find_here()) == 0;

    test_remote_arrays_on(0);
    test_remote_arrays_on(1);
    test_remote_arrays_chain(0, 0);
    test_remote_arrays_chain(0, 1);
    test_remote_arrays_chain(1, 0);
    test_remote_arrays_chain(1, 1);
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
        HPX_TEST_EQ(strm.str(),
            std::string("0\n1\n"
                        "[[42, 42], [42, 42]]\n[[42, 42], [42, 42]]\n[[42, "
                        "42], [42, 42]]\n"
                        "[[42, 42], [42, 42]]\n[[42, 42], [42, 42]]\n[[42, "
                        "42], [42, 42]]\n"
                        "[[42, 42], [42, 42]]\n[[42, 42], [42, 42]]\n"));
    }

    return hpx::util::report_errors();
}
