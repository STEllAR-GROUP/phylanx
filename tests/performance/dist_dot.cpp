// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/modules/testing.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
char const* const dot_product_code = R"(block(
    define(dot_product, dim_size,
        block(
            define(array1,
                random_d(list(dim_size, dim_size), find_here(), num_localities())),
            define(array2,
                random_d(list(dim_size, dim_size), find_here(), num_localities())),
            dot_d(array1, array2)
        )
    ),
    dot_product
))";

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    using namespace phylanx::execution_tree;

    // compile the given code
    compiler::function_list snippets;
    auto const& code_product = compile("dot_product", dot_product_code, snippets);
    auto dot_product = code_product.run();

    std::vector<std::int64_t> dim_sizes = {
        12, 120, 240, 480, 960, 4800, 9600, 12000};

    std::cout << "Having "
        << hpx::get_num_localities(hpx::launch::sync)
        << " localities:\n";

    for (std::int64_t const& dim_size : dim_sizes)
    {
        hpx::util::high_resolution_timer t;

        auto result = dot_product(dim_size);
        auto elapsed = t.elapsed();

        std::cout << "Result of Dot product for two square matrices of size "
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
