// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2017 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CROSS_OPERATION)
#define PHYLANX_PRIMITIVES_CROSS_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class cross_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<cross_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        cross_operation() = default;

        cross_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename);

    private:
        primitive_argument_type cross1d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;
        primitive_argument_type cross2d(
            primitive_argument_type&& lhs, primitive_argument_type&& rhs) const;

        template <typename T>
        primitive_argument_type cross1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type cross1d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type cross1d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type cross2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type cross2d1d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
        template <typename T>
        primitive_argument_type cross2d2d(
            ir::node_data<T>&& lhs, ir::node_data<T>&& rhs) const;
    };

    inline primitive create_cross_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "cross", std::move(operands), name, codename);
    }
}}}

#endif
