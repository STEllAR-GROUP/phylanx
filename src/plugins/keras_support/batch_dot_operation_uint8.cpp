// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/batch_dot_operation.hpp>
#include <phylanx/plugins/keras_support/batch_dot_operation_impl.hpp>

#include <cstddef>
#include <cstdint>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    // explicitly instantiate the required functions

    ///////////////////////////////////////////////////////////////////////////
    template primitive_argument_type batch_dot_operation::batch_dot2d(
        ir::node_data<std::uint8_t>&& lhs,
        ir::node_data<std::uint8_t>&& rhs) const;

    template primitive_argument_type batch_dot_operation::batch_dot2d(
        ir::node_data<std::uint8_t>&& lhs, ir::node_data<std::uint8_t>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const;

    template primitive_argument_type batch_dot_operation::batch_dot3d(
        ir::node_data<std::uint8_t>&& lhs,
        ir::node_data<std::uint8_t>&& rhs) const;

    template primitive_argument_type batch_dot_operation::batch_dot3d(
        ir::node_data<std::uint8_t>&& lhs, ir::node_data<std::uint8_t>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const;

    template primitive_argument_type batch_dot_operation::batch_dot4d(
        ir::node_data<std::uint8_t>&& lhs,
        ir::node_data<std::uint8_t>&& rhs) const;

    template primitive_argument_type batch_dot_operation::batch_dot4d(
        ir::node_data<std::uint8_t>&& lhs, ir::node_data<std::uint8_t>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const;

    template primitive_argument_type batch_dot_operation::batch_dot_nd(
        ir::node_data<std::uint8_t>&& lhs,
        ir::node_data<std::uint8_t>&& rhs) const;

    template primitive_argument_type batch_dot_operation::batch_dot_nd(
        ir::node_data<std::uint8_t>&& lhs, ir::node_data<std::uint8_t>&& rhs,
        std::size_t const& axis_a, std::size_t const& axis_b) const;

}}}

