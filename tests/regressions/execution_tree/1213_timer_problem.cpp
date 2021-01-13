// Copyright (c) 2020 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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
char const* const conv1d_code = R"(block(
    define(conv,
        block(
            define(array,
                random_d(list(8, 10, 2))),
            define(kernel,
                random(list(4, 2, 100))),
            timer(
                conv1d_d(array, kernel),
                lambda(time, cout("locality", find_here(), ":", time))
            )
        )
    ),
    conv
))";

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    using namespace phylanx::execution_tree;

    // compile the given code
    compiler::function_list snippets;
    auto const& compiled_code = compile("conv", conv1d_code, snippets);
    auto conv = compiled_code.run();


    conv();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = { "hpx.run_hpx_main!=1" };

    hpx::init_params params;
    params.cfg = std::move(cfg);
    return hpx::init(argc, argv, params);
}
