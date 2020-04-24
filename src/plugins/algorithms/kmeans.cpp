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
#include <numeric>
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
                    __arg(_5_seed, nil)
                )
            )"},
            &create_kmeans, &create_primitive<kmeans>, R"(
            points, num_centroids, iterations, num_points, show_result
            Args:

                points (matrix): a matrix with any number of rows and two
                    columns. Any data that can be represented by two features
                    can create the point matrix.
                num_centroids (int, optional): the number of clusters in which
                    we need to break down the data. It sets to 3 by default
                iterations (int, optional): the number of iterations. It sets
                    to 10 by default.
                show_result (bool, optional): defaults to false.

            Returns:

            Num ber of centroids points that shows the center of clusters given
            the point matrix.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    kmeans::kmeans(primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        // choose num_centroids random points as the initial centroids
        blaze::DynamicMatrix<double> initialize_centroids(
            blaze::DynamicMatrix<double> const& points, std::size_t num_points,
            std::size_t num_centroids)
        {
            blaze::DynamicMatrix<double> centroids(num_centroids, 2);
            std::uniform_int_distribution<std::int64_t> distribution(
                0, num_points - 1);
            std::vector<std::size_t> indices;
            std::int64_t rand_index;

            for (std::size_t i = 0; i != num_centroids; ++i)
            {
                rand_index = distribution(util::rng_);

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
        blaze::DynamicVector<std::size_t> closest_centroids(
            blaze::DynamicMatrix<double> const& points,
            blaze::DynamicMatrix<double> centroids,
            std::size_t num_points, std::size_t num_centroids)
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
        blaze::DynamicMatrix<double> move_centroids(
            blaze::DynamicMatrix<double> const& points,
            blaze::DynamicVector<std::size_t> closest,
            blaze::DynamicMatrix<double> centroids,
            std::size_t num_points, std::size_t num_centroids)
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
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type kmeans::calculate_kmeans(
        primitive_arguments_type&& args) const
    {
        // extract arguments
        auto arg0 = extract_numeric_value(args[0], name_, codename_);
        if (arg0.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "kmeans::calculate_kmeans",
                generate_error_message(
                    "the kmeans algorithm primitive requires for the first "
                    "argument, points, to represent a matrix"));
        }
        auto points = arg0.matrix();
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

        std::size_t seed = 13;
        if (valid(args[4]))
        {
            //seed = extract_scalar_positive_integer_value_strict(
            //    std::move(args[4]), name_, codename_);
        }
        util::set_seed(seed);

        std::size_t num_points = points.rows();
        blaze::DynamicMatrix<double> centroids =
            detail::initialize_centroids(points, num_points, num_centroids);

        blaze::DynamicVector<std::size_t> closest;
        for (std::size_t i = 0; i != iterations; ++i)
        {
            closest = detail::closest_centroids(
            points, centroids, num_points, num_centroids);
            centroids = detail::move_centroids(
                points, closest, centroids, num_points, num_centroids);
        }

        return primitive_argument_type{std::move(centroids)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> kmeans::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "kmeans::eval",
                generate_error_message(
                    "the kmeans algorithm primitive requires at least one and "
                    "at most 5 operands"));
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
