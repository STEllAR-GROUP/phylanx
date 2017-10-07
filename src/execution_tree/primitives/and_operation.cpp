//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/and_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/eigen.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::and_operation>
    and_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    and_operation_type, phylanx_and_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(and_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const and_operation::match_data =
    {
        "_1 && __2", &create<and_operation>
    };

    ///////////////////////////////////////////////////////////////////////////
    and_operation::and_operation(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and_operation::and_operation",
                "the and_operation primitive requires at least two operands");
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands_.size(); ++i)
        {
            if (!valid(operands_[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "and_operation::and_operation",
                "the and_operation primitive requires that the arguments given "
                    "by the operands array are valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // implement '&&' for all possible combinations of lhs and rhs
    hpx::future<util::optional<ir::node_data<double>>> and_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type && ops)
            {
                if (!detail::verify_argument_values(ops))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "and_operation::eval",
                        "the and_operation primitive requires that the argument"
                            " values given by the operands array are non-empty");
                }

                if (ops.size() == 2)
                {
                    return operand_type(ir::node_data<double>(
                            bool(ops[0].value()) && bool(ops[1].value()) ?
                                1.0 : 0.0
                        ));
                }

                return operand_type(ir::node_data<double>(
                    std::all_of(ops.begin(), ops.end(),
                        [](operand_type const& curr)
                        {
                            return bool(curr.value());
                        }) ? 1.0 : 0.0));
            }),
            detail::map_operands(operands_, evaluate_operand)
        );
    }
}}}
