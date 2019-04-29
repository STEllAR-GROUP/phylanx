// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_TILE_OPERATION
#define PHYLANX_TILE_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
/// \brief Constructs an array by repeating a, the number of times given by reps.
///
/// \param a     The scalar, vector, or matrix as the tile to repeat
/// \param reps  Integer or tuple of integers. Number of repetitions of a along
///              each axis
    class tile_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<tile_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        tile_operation() = default;

        tile_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        bool validate_reps(ir::range const& arg) const;

        primitive_argument_type tile0d(
            primitive_argument_type&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile0d_1arg(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile0d_2args(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile0d(ir::node_data<T>&& arr,
            ir::range&& arg) const;

        primitive_argument_type tile1d(
            primitive_argument_type&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile1d_1arg(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile1d_2args(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile1d(ir::node_data<T>&& arr,
            ir::range&& arg) const;

        primitive_argument_type tile2d(
            primitive_argument_type&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile2d_1arg(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile2d_2args(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile2d(ir::node_data<T>&& arr,
            ir::range&& arg) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type tile0d_3args(
            ir::node_data<T>&& arr, ir::range&& arg) const;

        template <typename T>
        primitive_argument_type tile1d_3args(
            ir::node_data<T>&& arr, ir::range&& arg) const;

        template <typename T>
        primitive_argument_type tile2d_3args(
            ir::node_data<T>&& arr, ir::range&& arg) const;


        primitive_argument_type tile3d(
            primitive_argument_type&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile3d_1arg(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile3d_2args(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile3d_3args(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type tile3d(ir::node_data<T>&& arr,
            ir::range&& arg) const;
#endif
    };

    inline primitive create_tile_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "tile", std::move(operands), name, codename);
    }
}}}

#endif
