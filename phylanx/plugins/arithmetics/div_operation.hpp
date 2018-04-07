// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_DIV_OPERATION_OCT_07_2017_0631PM)
#define PHYLANX_PRIMITIVES_DIV_OPERATION_OCT_07_2017_0631PM

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
    class div_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<div_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        using operand_type = ir::node_data<double>;
        using operands_type = std::vector<operand_type>;

    public:
        static match_pattern_type const match_data;

        div_operation() = default;

        div_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& args) const override;

    private:
        primitive_argument_type div0d0d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div0d0d(operands_type&& ops) const;
        primitive_argument_type div0d1d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div0d2d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div0d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div0d(operands_type&& ops) const;

        primitive_argument_type div1d0d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div1d1d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div1d1d(operands_type&& ops) const;
        primitive_argument_type div1d2d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div1d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div1d(operands_type&& ops) const;

        primitive_argument_type div2d0d(
            operand_type&& lhs, operand_type&& rhss) const;
        primitive_argument_type div2d1d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div2d2d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div2d2d(operands_type&& ops) const;
        primitive_argument_type div2d(
            operand_type&& lhs, operand_type&& rhs) const;
        primitive_argument_type div2d(operands_type&& ops) const;
    };

    inline primitive create_div_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "__div", std::move(operands), name, codename);
    }
}}}

#endif
