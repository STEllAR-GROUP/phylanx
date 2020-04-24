// Copyright (c) 2017-2020 Hartmut Kaiser
// Copyright (c) 2020 Maxwell Reeser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/util/random.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
char const* const cannon_product_code = R"(block(
    define(cannon, dim_size,
        block(
            define(array1, random_d(list(dim_size, dim_size), find_here(), 2)),
            define(array2, random_d(list(dim_size, dim_size), find_here(), 2)),
            cannon_product(array1, array2)
        )
    ),
    cannon
))";

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(there);

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

void test_remote_add(hpx::id_type const& here, hpx::id_type const& there)
{
    std::uint32_t seed = phylanx::util::get_seed();

    // generate two random matrices
    auto load_data = compile_and_run(load_data_str, here);
    auto m1 = load_data(std::int64_t(4), std::int64_t(4));
    auto m2 = load_data(std::int64_t(4), std::int64_t(4));

    // add them locally only
    auto add_here = compile_and_run(add_str, here);
    auto expected = add_here(m1, m2);

    // tile both matrices
    using namespace phylanx::execution_tree;

    // compile the given code
    compiler::function_list snippets;
    auto const& code_cannon_product =
        compile("cannon", cannon_product_code, snippets);
    auto cannon = code_cannon_product.run();

    hpx::util::high_resolution_timer t;

    std::int64_t dim_size = 12;
    auto result = cannon(dim_size);

    auto elapsed = t.elapsed();

    std::cout << "Result: \n"
              << extract_numeric_value(result) << std::endl
              << "Calculated in: " << elapsed << " seconds" << std::endl;

    std::cout << "using seed: " << seed << "\n";
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
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
