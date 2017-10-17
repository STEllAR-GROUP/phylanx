//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/and_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
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
    std::vector<match_pattern_type> const and_operation::match_data =
    {
        hpx::util::make_tuple("and", "_1 && __2", &create<and_operation>)
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
    namespace detail
    {
        struct and_ : std::enable_shared_from_this<and_>
        {
            and_(std::vector<primitive_argument_type> const& operands)
              : operands_(operands)
            {}

        private:
            using operands_type = std::vector<std::uint8_t>;

        public:
            hpx::future<primitive_result_type> eval() const
            {
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type && ops)
                    {
                        if (ops.size() == 2)
                        {
                            return primitive_result_type(
                                ops[0] != 0 && ops[1] != 0);
                        }

                        return primitive_result_type(
                            std::all_of(
                                ops.begin(), ops.end(),
                                [](std::uint8_t curr)
                                {
                                    return curr != 0;
                                }));
                    }),
                    detail::map_operands(operands_, boolean_operand)
                );
            }

        private:
            std::vector<primitive_argument_type> operands_;
        };
    }

    // implement '&&' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> and_operation::eval() const
    {
        return std::make_shared<detail::and_>(operands_)->eval();
    }
}}}
