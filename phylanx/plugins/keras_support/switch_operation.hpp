//  Copyright (c) 2019 Bita Hasheminezhad
//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_KERAS_SUPPORT_SWITCH)
#define PHYLANX_PRIMITIVES_KERAS_SUPPORT_SWITCH

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

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
    class switch_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<switch_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& operands,
            primitive_arguments_type const& args,
            eval_context ctx) const override;

    public:
        static match_pattern_type const match_data;

        switch_operation() = default;

        switch_operation(primitive_arguments_type&& operand,
            std::string const& name, std::string const& codename);

    private:
        bool validate_shapes(std::size_t const& ndims_cond,
            std::size_t const& ndims_then,
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims_cond,
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims_then) const;

        primitive_argument_type switch0d(ir::node_data<std::uint8_t>&& cond,
            ir::node_data<double>&& then_expr,
            ir::node_data<double>&& else_expr) const;
        primitive_argument_type switch1d(ir::node_data<std::uint8_t>&& cond,
            ir::node_data<double>&& then_expr,
            ir::node_data<double>&& else_expr) const;
        primitive_argument_type switch2d(ir::node_data<std::uint8_t>&& cond,
            ir::node_data<double>&& then_expr,
            ir::node_data<double>&& else_expr) const;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        primitive_argument_type switch3d(ir::node_data<std::uint8_t>&& cond,
            ir::node_data<double>&& then_expr,
            ir::node_data<double>&& else_expr) const;
#endif
    };

    inline primitive create_switch_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "switch", std::move(operands), name, codename);
    }
}}}

#endif
