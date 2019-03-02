// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_RESHAPE_OPERATION
#define PHYLANX_RESHAPE_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
/// \brief Returns a new shape to the given array without changing its data.
///
/// \param a         The scalar, vector, or matrix to reshape
/// \param newshape  Integer or tuple of integers. The new shape should be
///                  compatible with the original shape (number of elements in
///                  both arrays are the same).If an integer, then the result
///                  will be a 1-D array of that length.The last parameter of
///                  the newshape can be - 1. In this case, the value is inferred
///                  from the length of the array and remaining dimensions.
    class reshape_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<reshape_operation>
    {
    public:
        enum reshape_mode
        {
            general_reshape,
            flatten_mode
        };

    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static std::vector<match_pattern_type> const match_data;

        reshape_operation() = default;

        reshape_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        bool validate_shape(std::size_t const& n, ir::range const& arg) const;

        primitive_argument_type reshape0d(
            primitive_argument_type&& arr, ir::range&& arg) const;
        primitive_argument_type reshape1d(
            primitive_argument_type&& arr, ir::range&& arg) const;
        primitive_argument_type reshape2d(
            primitive_argument_type&& arr, ir::range&& arg) const;


        template <typename T>
        primitive_argument_type reshape0d(ir::node_data<T>&& arr,
            ir::range&& arg) const;

        template <typename T>
        primitive_argument_type reshape1d_2d(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type reshape1d(
            ir::node_data<T>&& arr, ir::range&& arg) const;

        template <typename T>
        primitive_argument_type reshape2d_1d(ir::node_data<T>&& arr) const;
        template <typename T>
        primitive_argument_type reshape2d_2d(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type reshape2d(ir::node_data<T>&& arr,
            ir::range&& arg) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type reshape3d(
            primitive_argument_type&& arr, ir::range&& arg) const;

        template <typename T>
        primitive_argument_type reshape1d_3d(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type reshape2d_3d(
            ir::node_data<T>&& arr, ir::range&& arg) const;

        template <typename T>
        primitive_argument_type reshape3d_1d(ir::node_data<T>&& arr) const;
        template <typename T>
        primitive_argument_type reshape3d_2d(
            ir::node_data<T>&& arr, ir::range&& arg) const;
        template <typename T>
        primitive_argument_type reshape3d_3d(
            ir::node_data<T>&& arr, ir::range&& arg) const;

        template <typename T>
        primitive_argument_type reshape3d(ir::node_data<T>&& arr,
            ir::range&& arg) const;
#endif

        template <typename T>
        primitive_argument_type flatten2d(
            ir::node_data<T>&& arr, std::string order) const;

        template <typename T>
        primitive_argument_type flatten_nd(ir::node_data<T>&& arr) const;

        template <typename T>
        primitive_argument_type flatten_nd(
            ir::node_data<T>&& arr, std::string order) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        template <typename T>
        primitive_argument_type flatten3d(
            ir::node_data<T>&& arr, std::string order) const;
#endif

    private:
        reshape_mode mode_;

    };

    inline primitive create_reshape_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "reshape", std::move(operands), name, codename);
    }

    inline primitive create_flatten_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name = "",
        std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "flatten", std::move(operands), name, codename);
    }
}}}

#endif
