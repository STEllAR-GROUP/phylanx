// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0.0. (See accompanying
// file LICENSE_1_0.0.txt or copy at http://www.boost.org/LICENSE_1_0.0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <iostream>

#include <blaze/Math.h>
#include <boost/program_options.hpp>

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
        add_dim(slice_column(points, 0))
    ),
    define(points_y,
        add_dim(slice_column(points, 1))
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
        ), 0
    )
))

define(move_centroids, points, closest, centroids, block(
    fmap(lambda(k, block(
                define(x, closest == k),
                mean(points * add_dim(x), 1)
    )),
        range(shape(centroids, 0))
    )
))

define(kmeans, points, k, iterations, block(
    define(centroids, initialize_centroids(points, k)),
    for(define(i, 0), i < iterations, store(i, i + 1), block(
        store(centroids,
              apply(vstack,
                    move_centroids(points,
                                   closest_centroids(points, centroids),
                                   centroids)))
    )),
    centroids
))
)";

blaze::DynamicMatrix<double> generate_random(int centroids, int num_points)
{
    /*blaze::DynamicMatrix<double> result(num_points, 2ul);
    blaze::Rand<blaze::DynamicVector<double>> gen_1d{};
    auto const raw_shares = gen_1d.generate(num_points);
    double const raw_shares_sum = std::accumulate(
        raw_shares.begin(), raw_shares.end(), 0.);
    auto const shares = num_points * raw_shares / raw_shares_sum;*/

    blaze::Rand<blaze::DynamicMatrix<double>> gen_2d{};

    /*for (auto const& i: shares)
    {
        result += gen_2d.generate(i, 2ul);
        blaze::
    }


    return result;*/
    return gen_2d.generate(num_points, 2ul);
}

int hpx_main(boost::program_options::variables_map& vm)
{
    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& prog = phylanx::execution_tree::compile(kmeans_code, snippets);
    auto kmeans = prog.run();

    // evaluate generated execution tree
    std::int64_t centroids = vm["centroids"].as<std::int64_t>();
    std::int64_t iterations = vm["iterations"].as<int64_t>();
    std::int64_t num_points = vm["points"].as<int64_t>();

    auto points = generate_random(centroids, num_points);

    // Measure execution time
    hpx::util::high_resolution_timer timer;

    auto result = phylanx::execution_tree::extract_numeric_value(
        kmeans(points, centroids, iterations));

    auto elapsed = timer.elapsed();

    // Make sure all counters are properly initialized, don't reset current
    // counter values
    hpx::reinit_active_counters(false);

    std::cout << result
              << "\nTime: " << elapsed << " seconds"
              << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    boost::program_options::options_description desc("usage: als [options]");
    desc.add_options()
        ("centroids", boost::program_options::value<std::int64_t>()->default_value(3),
            "number of centroids(default: 3)")
        ("iterations", boost::program_options::value<std::int64_t>()->default_value(2),
            "number of iterations (default: 2)")
        ("points", boost::program_options::value<std::int64_t>()->default_value(250),
            "alpha (default: 250)");

    return hpx::init(desc, argc, argv);
}
