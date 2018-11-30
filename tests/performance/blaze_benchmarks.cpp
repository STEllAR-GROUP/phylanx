//   Copyright (c) 2018 Shahrzad Shirzad
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
#include <vector>

///////////////////////////////////////////////////////////////////////////////
std::string const randvec_str = R"(
    define(call, size, random(size, "uniform"))
    call
)";

std::string const bench_daxpy = R"(
    define(run, a, b, block(
        define(c, b + a * 3.0), c
    ))
    run
)";

std::string const bench_dvecdvecadd = R"(
    define(run, a, b, block(
        define(c, b + a), c
    ))
    run
)";

void benchmark(std::string const& name,
    phylanx::execution_tree::compiler::function_list& snippets,
    std::string const& codestr,
    std::vector<std::int64_t> const& vec_sizes)
{
    auto const& rand_code =
        phylanx::execution_tree::compile(randvec_str, snippets);
    auto rand = rand_code.run();

    auto const& bench_code =
        phylanx::execution_tree::compile(codestr, snippets);
    auto bench = bench_code.run();

    std::uint64_t t;
    std::cout << "\n" << name << ":\n" << std::endl;

    for (std::int64_t i : vec_sizes)
    {
        phylanx::ir::node_data<std::int64_t> vec_size(i);
        auto input_vec1 = rand(vec_size);
        auto input_vec2 = rand(vec_size);

        t = hpx::util::high_resolution_clock::now();
        bench(input_vec1, input_vec2);
        t = hpx::util::high_resolution_clock::now() - t;

        std::cout << i << "       " << (t / 1e3) << " microseconds\n";
    }
}

int main(int argc, char* argv[])
{
    std::vector<std::int64_t> array_sizes = {
        100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

    phylanx::execution_tree::compiler::function_list snippets;
    benchmark("Daxpy", snippets, bench_daxpy, array_sizes);
    benchmark("Dvecdvecadd", snippets, bench_dvecdvecadd, array_sizes);

    return 0;
}
