// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Parsa Amini
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_KMEANS_AS_PRIMITIVE)
#define PHYLANX_KMEANS_AS_PRIMITIVE

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/futures/future.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

namespace phylanx { namespace execution_tree { namespace primitives
{
    ///
    /// Creates a primitive executing the kmeans algorithm on the given
    /// input data
    ///
    class kmeans
      : public primitive_component_base
      , public std::enable_shared_from_this<kmeans>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        kmeans() = default;


        kmeans(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    protected:
        blaze::DynamicMatrix<double> initialize_centroids(
            blaze::DynamicMatrix<double> const& points, std::size_t num_points,
            std::size_t num_centroids) const;
        blaze::DynamicVector<std::size_t> closest_centroids(
            blaze::DynamicMatrix<double> const& points,
            blaze::DynamicMatrix<double> const& centroids,
            std::size_t num_points, std::size_t num_centroids) const;
        blaze::DynamicMatrix<double> move_centroids(
            blaze::DynamicMatrix<double> const& points,
            blaze::DynamicVector<std::size_t>&& closest,
            blaze::DynamicMatrix<double>&& centroids, std::size_t num_points,
            std::size_t num_centroids) const;

        primitive_argument_type calculate_kmeans(
            primitive_arguments_type&& args) const;
    };

    inline primitive create_kmeans(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "kmeans", std::move(operands), name, codename);
    }
}}}

#endif
