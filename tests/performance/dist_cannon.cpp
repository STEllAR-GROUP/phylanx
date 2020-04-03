// Copyright (c) 2017-2020 Hartmut Kaiser
// Copyright (c) 2020 Maxwell Reeser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
char const* const cannon_product_code = R"(block(
    define(cannon, dim_size,
        block(
            define(array1,
                random_d(list(dim_size, dim_size), find_here(), num_localities())),
            define(array2,
                random_d(list(dim_size, dim_size), find_here(), num_localities())),
            cannon_product(array1, array2)
        )
    ),
    cannon
))";

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    using namespace phylanx::execution_tree;

    // compile the given code
    compiler::function_list snippets;
    auto const& code_product = compile("cannon", cannon_product_code, snippets);
    auto cannon = code_product.run();

    std::vector<std::int64_t> dim_sizes = {
        12, 120, 240, 480, 960, 4800, 9600, 96000, 960000};

    std::cout << "Having " << hpx::get_num_localities().get() << " localities:\n";

    for (std::int64_t const& dim_size : dim_sizes)
    {
        hpx::util::high_resolution_timer t;

        auto result = cannon(dim_size);
        auto elapsed = t.elapsed();

        std::cout << "Result of Cannon product for two square matrices of size "
            << dim_size
            << "\n on locality "
            << hpx::get_locality_id()
            << " is calculated in: " << elapsed << " seconds" << std::endl;
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
