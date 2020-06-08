// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2010 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0.0. (See accompanying
// file LICENSE_1_0.0.txt or copy at http://www.boost.org/LICENSE_1_0.0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <blaze/Math.h>
#include <hpx/program_options.hpp>

#include <hpx/include/agas.hpp>
#include <hpx/runtime_fwd.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
char const* const kmeans_code = R"(
    define(closest_centroids, points, centroids, block(
        define(expanded_centroids,
            reshape(centroids, list(shape(centroids, 0), 1, 2))
        ),
        define(distances,
            sqrt(sum(square(points - expanded_centroids), 2))
        ),
        argmin(distances, 0)
    ))

    define(move_centroids, points, closest, centroids, block(
        fmap(lambda(k, block(
                    define(x, closest == k),
                    define(count, sum(x * constant(1, shape(x, 0)))),
                    define(sum_, sum(points * expand_dims(x, -1), 0)),
                    sum_/count
            )),
            range(shape(centroids, 0))
        )
    ))

    define(kmeans, points, iterations, initial_centroids, enable_output,
        block(
            define(centroids, initial_centroids),
            for(define(i, 0), i < iterations, store(i, i + 1),
                block(
                    if(enable_output, block(
                        cout("centroids in iteration ", i,": ", centroids)
                    )),
                    store(centroids,
                          apply(vstack,
                                list(move_centroids(points,
                                        closest_centroids(points, centroids),
                                        centroids))))
                )
            ), centroids
        )
    )
)";

int hpx_main(hpx::program_options::variables_map& vm)
{
    blaze::DynamicMatrix<double> points{{0.75, 0.25}, {1.25, 3.}, {2.75, 3.},
        {1., 0.25}, {3., 0.25}, {1.5, 2.75}, {3., 0.}, {3., 3.}, {2.75, 2.25},
        {2.5, 1.75}, {11., 4.5}, {10.5, 3.}, {9.5, 5.}, {10., 3.5},
        {11.25, 6.25}, {9.25, 3.}, {11.75, 0.75}, {10., 2.75}, {12., 5.75},
        {15.25, 2.25}, {-3., 14.75}, {4.75, 10.25}, {-1.25, 13.25},
        {-0.25, 13.}, {0.75, 9.25}, {-0.25, 9.25}, {2.5, 8.75}, {-2.25, 10.25},
        {-2.25, 10.75}, {2.5, 11.75}};

    blaze::DynamicMatrix<double> initial_centroids{
        {-3., 14.75}, {1.5, 2.75}, {3., 0.}};

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& prog = phylanx::execution_tree::compile(kmeans_code, snippets);

    phylanx::execution_tree::eval_context ctx;
    auto kmeans = prog.run(ctx);

    // evaluate generated execution tree
    std::int64_t iterations = vm["iterations"].as<int64_t>();
    bool enable_output = vm["enable_output"].as<bool>();


    // Measure execution time
    hpx::util::high_resolution_timer timer;

    auto result = phylanx::execution_tree::extract_numeric_value(
        kmeans(ctx, points, iterations, initial_centroids, enable_output));

    auto elapsed = timer.elapsed();

    // Make sure all counters are properly initialized, don't reset current
    // counter values
    hpx::reinit_active_counters(false);

    std::cout << "Centroids are: " << result << std::endl;
    std::cout << "Calculated in: " << elapsed << " seconds" << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    hpx::program_options::options_description desc("usage: kmeans [options]");
    desc.add_options()
        ("iterations, i", hpx::program_options::value<std::int64_t>()->default_value(5),
            "number of iterations (default: 5)")
        ("enable_output, e", hpx::program_options::value<bool>()->default_value(false),
            "enable progress output (default: false)");

    return hpx::init(desc, argc, argv);
}
