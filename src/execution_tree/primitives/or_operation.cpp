//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/or_operation.hpp>
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
    std::vector<match_pattern_type> const or_operation::match_data =
    {
        hpx::util::make_tuple("or", "_1 || __2", &create<or_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    or_operation::or_operation(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct or_ : std::enable_shared_from_this<or_>
        {
            or_() = default;

        private:
            using operands_type = std::vector<std::uint8_t>;

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() < 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::eval",
                        "the or_operation primitive requires at least "
                            "two operands");
                }

                bool arguments_valid = true;
                for (std::size_t i = 0; i != operands.size(); ++i)
                {
                    if (!valid(operands[i]))
                    {
                        arguments_valid = false;
                    }
                }

                if (!arguments_valid)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "or_operation::eval",
                        "the or_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type && ops)
                    {
                        if (ops.size() == 2)
                        {
                            return primitive_result_type(
                                ops[0] != 0 || ops[1] != 0);
                        }

                        return primitive_result_type(
                            std::any_of(
                                ops.begin(), ops.end(),
                                [](std::uint8_t curr)
                                {
                                    return curr != 0;
                                }));
                    }),
                    detail::map_operands(operands, boolean_operand, args)
                );
            }
        };
    }

    // implement '||' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> or_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::or_>()->eval(args, {});
        }

        return std::make_shared<detail::or_>()->eval(operands_, args);
    }
}}}
