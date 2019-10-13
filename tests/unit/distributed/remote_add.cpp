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
#include <hpx/include/runtime.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
std::string load_data_str = R"(
    define(load_data, rows, cols, random(list(rows, cols)))
)";

std::string add_str = R"(
    define(add, a, b, a + b)
)";

std::string columnwise_tile_str = R"(
    define(columnwise_tile, m, n, hsplit(m, n))
)";

std::string columnwise_merge_str = R"(
    define(columnwise_merge, tiles, hstack(tiles))
)";

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile_and_run(
    std::string const& codestr, hpx::id_type const& there)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(there);

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

void test_remote_add(hpx::id_type const& here, hpx::id_type const& there)
{
    // generate two random matrices
    auto load_data = compile_and_run(load_data_str, here);
    auto m1 = load_data(std::int64_t(4), std::int64_t(4));
    auto m2 = load_data(std::int64_t(4), std::int64_t(4));

    // add them locally only
    auto add_here = compile_and_run(add_str, here);
    auto expected = add_here(m1, m2);

    // tile both matrices
    using namespace phylanx::execution_tree;

    auto columnwise_tile = compile_and_run(columnwise_tile_str, here);
    auto m1_tiles =
        extract_list_value(columnwise_tile(m1, std::int64_t(2))).args();
    auto m2_tiles =
        extract_list_value(columnwise_tile(m2, std::int64_t(2))).args();

    // perform remote addition
    auto result1 = add_here.eval(m1_tiles[0], m2_tiles[0]);

    auto add_there = compile_and_run(add_str, there);
    auto result2 = add_there.eval(m1_tiles[1], m2_tiles[1]);

    // wait for add operations to finish
    hpx::wait_all(result1, result2);

    // merge tiles
    auto columnwise_merge = compile_and_run(columnwise_merge_str, here);
    auto result = columnwise_merge(primitive_argument_type{
        primitive_arguments_type{result1.get(), result2.get()}});

    HPX_TEST_EQ(expected, result);
}

int hpx_main(int argc, char* argv[])
{
    std::vector<hpx::id_type> localities = hpx::find_all_localities();
    HPX_TEST(localities.size() >= 2);

    test_remote_add(localities[0], localities[1]);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);
    return hpx::util::report_errors();
}
