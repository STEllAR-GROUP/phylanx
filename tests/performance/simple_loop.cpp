//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #244: Can not create a list or a vector of previously defined variables

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

///////////////////////////////////////////////////////////////////////////////
std::string const randstr = R"(
    define(call, size, random(size, "uniform"))
    call
)";

std::string const bench1 = R"(
    define(run, y, k, block(
        define(x, constant(0.0, k)),
        define(local_y, y),
        define(z, 0),
        for_each(
            lambda(i, store(z, slice(x, slice(local_y, i, nil)) + 1, nil)),
            range(k)
        )
    ))
    run
)";

std::string const bench2 = R"(
    define(run, y, k, block(
        define(x, constant(0.0, k)),
        define(local_y, y),
        for_each(
            lambda(i, block(
                define(idx, slice(local_y, i, nil)),
                store(slice(x, idx, nil), slice(x, idx, nil) + 1)
            )),
            range(k)
        )
    ))
    run
)";

#define ARRAY_SIZE std::int64_t(100000)

///////////////////////////////////////////////////////////////////////////////
template <typename Data>
void benchmark(std::string const& name,
    phylanx::execution_tree::compiler::function_list& snippets,
    std::string const& codestr, Data const& y)
{
    auto const& code = phylanx::execution_tree::compile(codestr, snippets);
    auto bench = code.run();


    std::uint64_t t = hpx::util::high_resolution_clock::now();

    bench(y, ARRAY_SIZE);

    t = hpx::util::high_resolution_clock::now() - t;

    std::cout << name << ": " << (t / 1e6) << " ms.\n";
}

int main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& rand_code = phylanx::execution_tree::compile(randstr, snippets);
    auto rand = rand_code.run();

    auto y = rand(ARRAY_SIZE);

    benchmark("bench1", snippets, bench1, y);
    benchmark("bench2", snippets, bench2, y);

    return 0;
}

