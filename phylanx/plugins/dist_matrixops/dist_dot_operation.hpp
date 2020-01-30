//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIST_DOT_OPERATION_JUN_17_2019_0657AM)
#define PHYLANX_PRIMITIVES_DIST_DOT_OPERATION_JUN_17_2019_0657AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace phylanx { namespace dist_matrixops { namespace primitives
{
    class dist_dot_operation
      : public execution_tree::primitives::primitive_component_base
      , public std::enable_shared_from_this<dist_dot_operation>
    {
    protected:
        hpx::future<execution_tree::primitive_argument_type> eval(
            execution_tree::primitive_arguments_type const& operands,
            execution_tree::primitive_arguments_type const& args,
            execution_tree::eval_context ctx) const override;

    public:
        static execution_tree::match_pattern_type const match_data;

        dist_dot_operation() = default;

        dist_dot_operation(execution_tree::primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        template <typename T>
        execution_tree::primitive_argument_type dot0d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        execution_tree::primitive_argument_type dot0d(
            execution_tree::primitive_argument_type&&,
            execution_tree::primitive_argument_type&&) const;

        template <typename T>
        execution_tree::primitive_argument_type dot1d1d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs,
            execution_tree::localities_information&& lhs_localities,
            execution_tree::localities_information const& rhs_localities) const;
        template <typename T>
        execution_tree::primitive_argument_type dot1d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
            execution_tree::localities_information&& lhs_localities,
            execution_tree::localities_information const& rhs_localities) const;
        template <typename T>
        execution_tree::primitive_argument_type dot1d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        execution_tree::primitive_argument_type dot1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
            execution_tree::localities_information&& lhs_localities,
            execution_tree::localities_information const& rhs_localities) const;
        execution_tree::primitive_argument_type dot1d(
            execution_tree::primitive_argument_type&&,
            execution_tree::primitive_argument_type&&) const;

        template <typename T>
        execution_tree::primitive_argument_type dot2d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
            execution_tree::localities_information&& lhs_localities,
            execution_tree::localities_information const& rhs_localities) const;
        template <typename T>
        execution_tree::primitive_argument_type dot2d2d(ir::node_data<T>&& lhs,
            ir::node_data<T>&& rhs,
            execution_tree::localities_information&& lhs_localities,
            execution_tree::localities_information const& rhs_localities) const;
        template <typename T>
        execution_tree::primitive_argument_type dot2d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        execution_tree::primitive_argument_type dot2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
            execution_tree::localities_information&& lhs_localities,
            execution_tree::localities_information const& rhs_localities) const;
        execution_tree::primitive_argument_type dot2d(
            execution_tree::primitive_argument_type&&,
            execution_tree::primitive_argument_type&&) const;

        template <typename T>
        execution_tree::primitive_argument_type dot3d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        execution_tree::primitive_argument_type dot3d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        execution_tree::primitive_argument_type dot3d3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        execution_tree::primitive_argument_type dot3d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs,
            execution_tree::localities_information const& lhs_localities,
            execution_tree::localities_information const& rhs_localities) const;
        execution_tree::primitive_argument_type dot3d(
            execution_tree::primitive_argument_type&&,
            execution_tree::primitive_argument_type&&) const;

        execution_tree::primitive_argument_type dot_nd(
            execution_tree::primitive_argument_type&& lhs,
            execution_tree::primitive_argument_type&& rhs) const;
    };

    inline execution_tree::primitive create_dist_dot_operation(
        hpx::id_type const& locality,
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return execution_tree::create_primitive_component(
            locality, "dot_d", std::move(operands), name, codename);
    }
}}}

#endif
