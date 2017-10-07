//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/or_operation.hpp>
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
    phylanx::execution_tree::primitives::or_operation>
    or_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    or_operation_type, phylanx_or_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(or_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const or_operation::match_data =
    {
        "_1 || __2", &create<or_operation>
    };

    ///////////////////////////////////////////////////////////////////////////
    or_operation::or_operation(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "or_operation::or_operation",
                "the or_operation primitive requires at least two operors");
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
                "or_operation::or_operation",
                "the or_operation primitive requires that the arguments given "
                    "by the operands array are valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // implement '||' for all possible combinations of lhs or rhs
    hpx::future<primitive_result_type> or_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type && ops)
            {
                if (ops.size() == 2)
                {
                    return primitive_result_type(ops[0] || ops[1]);
                }

                return primitive_result_type(
                    std::any_of(ops.begin(), ops.end(),
                        [](std::uint8_t curr) { return curr; }));
            }),
            detail::map_operands(operands_, boolean_operand)
        );
    }
}}}
