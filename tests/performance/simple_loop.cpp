//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

#define ARRAY_SIZE std::int64_t(100000)

///////////////////////////////////////////////////////////////////////////////
std::string const randstr = R"(
    define(call, size, random(size, list("uniform_int", 0, 99999)))
    call
)";

std::string const bench1 = R"(
    define(run, y, k, block(
        define(x, constant(0.0, k)),
        define(local_y, y),
        define(z, 0),
        for_each(
            lambda(i, store(z, slice(x, slice(local_y, i)) + 1)),
            range(k)
        ),
        z
    ))
    run
)";

std::string const bench1_intidx = R"(
    define(run, y, k, block(
        define(x, constant(0.0, k)),
        define(local_y, y),
        define(z, 0),
        store(z, slice(x, local_y) + 1),
        z
    ))
    run
)";

std::string const bench2 = R"(
    define(run, y, k, block(
        define(x, constant(0.0, k)),
        define(local_y, y),
        for_each(
            lambda(i, block(
                define(idx, slice(local_y, i)),
                store(slice(x, idx), slice(x, idx) + 1)
            )),
            range(k)
        ),
        x
    ))
    run
)";

std::string const bench2_intidx = R"(
    define(run, y, k, block(
        define(x, constant(0.0, k)),
        define(local_y, y),
        store(slice(x, local_y), slice(x, local_y) + 1),
        x
    ))
    run
)";

///////////////////////////////////////////////////////////////////////////////
template <typename Data>
void benchmark(std::string const& name,
    phylanx::execution_tree::compiler::function_list& snippets,
    std::string const& codestr, Data const& y)
{
    auto const& code = phylanx::execution_tree::compile(codestr, snippets);
    auto bench = code.run();


    std::uint64_t t = hpx::chrono::high_resolution_clock::now();

    bench(y, ARRAY_SIZE);

    t = hpx::chrono::high_resolution_clock::now() - t;

    std::cout << name << ": " << (t / 1e6) << " ms.\n";
}

int main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& rand_code = phylanx::execution_tree::compile(randstr, snippets);
    auto rand = rand_code.run();

    phylanx::execution_tree::primitive_arguments_type dims{
        phylanx::execution_tree::primitive_argument_type(ARRAY_SIZE)};

    auto y = rand(dims);

    benchmark("bench1", snippets, bench1, y);
    benchmark("bench2", snippets, bench2, y);

    benchmark("bench1_intidx", snippets, bench1_intidx, y);
    benchmark("bench2_intidx", snippets, bench2_intidx, y);

    return 0;
}

