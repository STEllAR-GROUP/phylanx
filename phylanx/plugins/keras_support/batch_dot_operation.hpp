// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_BATCH_DOT_OPERATION)
#define PHYLANX_PRIMITIVES_BATCH_DOT_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class batch_dot_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<batch_dot_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        batch_dot_operation() = default;

        batch_dot_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        std::size_t positivize_axis(std::int64_t axis, std::size_t const& ndim) const;
        bool validate_axes(ir::range const& axes, std::size_t&& ndims_a,
            std::size_t&& ndims_b,
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims_a,
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims_b,
            std::size_t& axis_a, std::size_t& axis_b) const;

        template <typename T>
        primitive_argument_type batch_dot2d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, std::size_t const& axis_a,
            std::size_t const& axis_b) const;

        template <typename T>
        primitive_argument_type batch_dot_nd(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot_nd(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, std::size_t const& axis_a,
            std::size_t const& axis_b) const;

        template <typename T>
        primitive_argument_type batch_dot2d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot2d3d_axes12(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot2d4d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot2d4d_axis1(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot2d4d_axes13(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot2d3d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, std::size_t const& axis_a,
            std::size_t const& axis_b) const;
        template <typename T>
        primitive_argument_type batch_dot2d4d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, std::size_t const& axis_a,
            std::size_t const& axis_b) const;

        template <typename T>
        primitive_argument_type batch_dot3d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d2d_axis1(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, std::size_t const& axis_a,
            std::size_t const& axis_b) const;

        template <typename T>
        primitive_argument_type batch_dot3d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d3d_axis1(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d3d_axis2(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d3d_axes12(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d3d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, std::size_t const& axis_a,
            std::size_t const& axis_b) const;

        template <typename T>
        primitive_argument_type batch_dot3d4d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d4d_axis1(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d4d_axes12(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d4d_axes13(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d4d_axes21(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d4d_axes23(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d4d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, std::size_t const& axis_a,
            std::size_t const& axis_b) const;

        template <typename T>
        primitive_argument_type batch_dot3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type batch_dot3d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs, std::size_t const& axis_a,
            std::size_t const& axis_b) const;
    };

    inline primitive create_batch_dot_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "batch_dot", std::move(operands), name, codename);
    }
}}}

#endif
