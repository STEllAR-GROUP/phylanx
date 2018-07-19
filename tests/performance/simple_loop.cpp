//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #244: Can not create a list or a vector of previously defined variables

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <iostream>
#include <string>

///////////////////////////////////////////////////////////////////////////////
std::string const randstr = R"(
    define(call, size, random(size, "uniform"))
    call
)";

std::string const codestr = R"(
    define(run, y, k, block(
        define(x, constant(0.0, k)),
        define(z, 0),
        map(
            lambda(i, store(z, slice(x, slice(y, i)) + 1)),
            range(k)
        )
    ))
    run
)";

#define ARRAY_SIZE 100000ll

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& rand_code = phylanx::execution_tree::compile(randstr, snippets);
    auto rand = rand_code.run();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets);
    auto bench = code.run();

    {
        auto y = rand(ARRAY_SIZE);

        std::uint64_t t = hpx::util::high_resolution_clock::now();

        bench(std::move(y), ARRAY_SIZE);

        t = hpx::util::high_resolution_clock::now() - t;

        std::cout << "elapsed time: " << (t / 1e6) << " ms.\n";
    }
    return 0;
}

