// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
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
    define(initialize_centroids, points, k, block(
        define(centroids, points),
        shuffle(centroids),
        slice(centroids, make_list(0, k))
    ))

    define(closest_centroids, points, centroids, block(
        define(points_x,
            expand_dims(slice_column(points, 0), -1)
        ),
        define(points_y,
            expand_dims(slice_column(points, 1), -1)
        ),
        define(centroids_x,
            slice_column(centroids, 0)
        ),
        define(centroids_y,
            slice_column(centroids, 1)
        ),
        argmin(sqrt(
                    power(points_x - centroids_x, 2
                    )+power(points_y - centroids_y, 2)
            ), 1
        )
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

    define(__kmeans, points, k, iterations, block(
        define(centroids, initialize_centroids(points, k)),
        for(define(i, 0), i < iterations, store(i, i + 1), block(
            store(centroids,
                  apply(vstack,
                        list(move_centroids(points,
                                       closest_centroids(points, centroids),
                                       centroids))))
        )),
        centroids
    ))
)";

blaze::DynamicMatrix<double> generate_random(int num_points)
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen_2d{};

    return gen_2d.generate(num_points, 2ul);
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& prog = phylanx::execution_tree::compile(kmeans_code, snippets);

    phylanx::execution_tree::eval_context ctx;
    auto kmeans = prog.run(ctx);

    // evaluate generated execution tree
    std::int64_t centroids = vm["centroids"].as<std::int64_t>();
    std::int64_t iterations = vm["iterations"].as<int64_t>();
    std::int64_t num_points = vm["points"].as<int64_t>();
    bool show_result = vm["show_result"].as<bool>();

    auto points = generate_random(num_points);

    // Measure execution time
    hpx::chrono::high_resolution_timer timer;

    auto result = phylanx::execution_tree::extract_numeric_value(
        kmeans(ctx, points, centroids, iterations));

    auto elapsed = timer.elapsed();

    // Make sure all counters are properly initialized, don't reset current
    // counter values
    hpx::reinit_active_counters(false);

    if (show_result)
        std::cout << result << "\n";

    std::cout << "Time: " << elapsed << " seconds"
              << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    hpx::program_options::options_description desc("usage: kmeans [options]");
    desc.add_options()
        ("centroids", hpx::program_options::value<std::int64_t>()->default_value(3),
            "number of centroids(default: 3)")
        ("iterations", hpx::program_options::value<std::int64_t>()->default_value(2),
            "number of iterations (default: 2)")
        ("points", hpx::program_options::value<std::int64_t>()->default_value(250),
            "number of points (default: 250)")
        ("show_result", hpx::program_options::value<bool>()->default_value(false),
            "show calculated result (default: false)");

    return hpx::init(desc, argc, argv);
}
