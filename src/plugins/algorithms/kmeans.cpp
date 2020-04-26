// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Parsa Amini
// Copyright (c) 2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/algorithms/kmeans.hpp>
#include <phylanx/util/random.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const kmeans::match_data =
    {
        hpx::util::make_tuple("kmeans",
        std::vector<std::string>{R"(
                kmeans(
                    _1_points,
                    __arg(_2_num_centroid, 3),
                    __arg(_3_iterations, 10),
                    __arg(_4_show_result, false),
                    __arg(_5_seed, nil),
                    __arg(_6_initial_centroids, nil)
                )
            )"},
            &create_kmeans, &create_primitive<kmeans>, R"(
            points, num_centroids, iterations, show_result, seed,
            initial_centroids

            Args:

                points (matrix): a matrix with any number of rows and two
                    columns. Any data that can be represented by two features
                    can create the point matrix.
                num_centroids (int, optional): the number of clusters in which
                    we need to break down the data. It sets to 3 by default
                iterations (int, optional): the number of iterations. It sets
                    to 10 by default.
                show_result (bool, optional): defaults to false.
                seed (int) : the seed of a random number generator.
                initial_centroids (matrix): if not given, the centroids are
                    initialized by num_centroids randomly chosen points. If
                    given there is no use for a seed. The initial_centroids
                    matrix should have num_centroids rows and 2 columns.

            Returns:

            Number of centroids points that shows the center of clusters given
            the points matrix.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    kmeans::kmeans(primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    // choose num_centroids random points as the initial centroids
    blaze::DynamicMatrix<double> kmeans::initialize_centroids(
        blaze::DynamicMatrix<double> const& points, std::size_t num_points,
        std::size_t num_centroids) const
    {
        blaze::DynamicMatrix<double> centroids(num_centroids, 2);
        std::uniform_int_distribution<std::int64_t> distribution(
            0, num_points - 1);
        std::vector<std::size_t> indices;
        std::int64_t rand_index;

        for (std::size_t i = 0; i != num_centroids; ++i)
        {
            rand_index = distribution(util::rng_);

            // rand indices should be unique
            while (std::find(indices.begin(), indices.end(), rand_index) !=
                indices.end())
            {
                rand_index = distribution(util::rng_);
            }
            indices.emplace_back(rand_index);

            blaze::row(centroids, i) = blaze::row(points, rand_index);
        }
        return std::move(centroids);
    }

    // assign each point to its closest centroid
    blaze::DynamicVector<std::size_t> kmeans::closest_centroids(
        blaze::DynamicMatrix<double> const& points,
        blaze::DynamicMatrix<double> const& centroids, std::size_t num_points,
        std::size_t num_centroids) const
    {
        auto points_x = blaze::column(points, 0);
        auto points_y = blaze::column(points, 1);
        auto centroids_x = blaze::column(centroids, 0);
        auto centroids_y = blaze::column(centroids, 1);

        // calculating the index for each row of points
        blaze::DynamicVector<std::size_t> result(num_points);
        for (std::size_t i = 0; i != num_points; ++i)
        {
            auto temp = blaze::DynamicVector<double>(num_centroids);
            for (std::size_t j = 0; j != num_centroids; ++j)
            {
                temp[j] = blaze::sqrt(
                    blaze::pow(points_x[i] - centroids_x[j], 2) +
                    blaze::pow(points_y[i] - centroids_y[j], 2));
            }
            result[i] = blaze::argmin(temp);
        }
        return std::move(result);
    }

    // generates new centroids as the centers of clusters
    blaze::DynamicMatrix<double> kmeans::move_centroids(
        blaze::DynamicMatrix<double> const& points,
        blaze::DynamicVector<std::size_t>&& closest,
        blaze::DynamicMatrix<double>&& centroids,
        std::size_t num_points, std::size_t num_centroids) const
    {
        blaze::DynamicMatrix<double> result(num_centroids, 2, 0.0);
        std::size_t count;
        for (size_t k = 0; k != num_centroids; ++k)
        {
            count = 0;
            for (size_t i = 0; i != num_points; ++i)
            {
                if (closest[i] == k)
                {
                    blaze::row(result, k) += blaze::row(points, i);
                    count += 1;
                }
            }
            if (count != 0)
            {
                blaze::row(result, k) /= count;
            }
        }
        return std::move(result);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type kmeans::calculate_kmeans(
        primitive_arguments_type&& args) const
    {
        // extract arguments
        auto arg0 = extract_numeric_value(std::move(args[0]), name_, codename_);
        if (arg0.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "kmeans::calculate_kmeans",
                generate_error_message(
                    "the kmeans algorithm primitive requires for the first "
                    "argument, points, to represent a matrix"));
        }
        auto const points = arg0.matrix();
        if (points.columns() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "kmeans::calculate_kmeans",
                generate_error_message(
                    "the kmeans algorithm primitive requires for the first "
                    "argument, points, to have exacty 2 columns"));
        }

        std::size_t num_centroids = 3;
        if (valid(args[1]))
        {
            num_centroids = extract_scalar_positive_integer_value_strict(
                std::move(args[1]), name_, codename_);
        }

        std::size_t iterations = 10;
        if (valid(args[2]))
        {
            iterations = extract_scalar_positive_integer_value_strict(
                std::move(args[2]), name_, codename_);
        }

        bool show_result = false;
        if (valid(args[3]))
        {
            show_result = extract_scalar_boolean_value(
                std::move(args[3]), name_, codename_);
        }

        std::uint32_t seed = 42;
        if (valid(args[4]))
        {
            seed = extract_scalar_positive_integer_value_strict(
                std::move(args[4]), name_, codename_);
        }
        util::set_seed(seed);

        std::size_t num_points = points.rows();

        // initializing the centroids
        blaze::DynamicMatrix<double> centroids;
        if (valid(args[5]))
        {
            auto arg5 =
                extract_numeric_value(std::move(args[5]), name_, codename_);
            if (arg5.num_dimensions() != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "kmeans::calculate_kmeans",
                    generate_error_message(
                        "the kmeans algorithm primitive requires for the "
                        "initial_centroids to represent a matrix"));
            }
            centroids = arg5.matrix();
            if (centroids.columns() != 2 || centroids.rows() != num_centroids)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "kmeans::calculate_kmeans",
                    generate_error_message(
                        "the kmeans algorithm primitive requires for the "
                        "initial_centroids to have num_centroids rows and 2 "
                        "columns"));
            }
        }
        else
        {
            centroids = initialize_centroids(points, num_points, num_centroids);
        }

        // kmeans calculations
        blaze::DynamicVector<std::size_t> closest;
        for (std::size_t i = 0; i != iterations; ++i)
        {
            closest =
                closest_centroids(points, centroids, num_points, num_centroids);
            centroids = move_centroids(points, std::move(closest),
                std::move(centroids), num_points, num_centroids);
            if (show_result)
            {
                std::cout << "centroids after iteration " << i << ": "
                          << centroids << std::endl;
            }
        }

        return primitive_argument_type{std::move(centroids)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> kmeans::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 6)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "kmeans::eval",
                generate_error_message(
                    "the kmeans algorithm primitive requires at least one and "
                    "at most 6 operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "kmeans::eval",
                generate_error_message(
                    "the kmeans algorithm primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_arguments_type&& args)
                    -> primitive_argument_type
                {
                    return this_->calculate_kmeans(std::move(args));
                }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_,
                std::move(ctx)));
    }
}}}
