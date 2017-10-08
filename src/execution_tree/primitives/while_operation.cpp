//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/while_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::while_operation>
    while_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    while_operation_type, phylanx_while_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(while_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const while_operation::match_data =
    {
        "while(_1, _2)", &create<while_operation>
    };

    ///////////////////////////////////////////////////////////////////////////
    while_operation::while_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::while_operation::"
                    "while_operation",
                "the while_operation primitive requires exactly two arguments");
        }

        if (!valid(operands_[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::while_operation::"
                    "while_operation",
                "the while_operation primitive requires that the arguments "
                    "given by the operands array are valid");
        }
    }

    namespace detail
    {
        struct iteration : std::enable_shared_from_this<iteration>
        {
            iteration(std::vector<primitive_argument_type> const& operands)
              : operands_(operands)
              , result_(hpx::make_ready_future(primitive_result_type{}))
            {}

            hpx::future<primitive_result_type> body(
                hpx::future<primitive_result_type>&& cond)
            {
                if (extract_boolean_value(cond.get()))
                {
                    // evaluate body of while statement
                    auto this_ = this->shared_from_this();
                    return literal_operand(operands_[1]).then(
                        [this_](
                            hpx::future<primitive_result_type> && result
                        ) mutable
                        {
                            this_->result_ = std::move(result);
                            return this_->loop();
                        });
                }
                return std::move(result_);
            }

            hpx::future<primitive_result_type> loop()
            {
                // evaluate condition of while statement
                auto this_ = this->shared_from_this();
                return literal_operand(operands_[0]).then(
                    [this_](hpx::future<primitive_result_type> && cond)
                    {
                        return this_->body(std::move(cond));
                    });
            }

        private:
            std::vector<primitive_argument_type> operands_;
            hpx::future<primitive_result_type> result_;
        };
    }

    // start iteration over given while statement
    hpx::future<primitive_result_type> while_operation::eval() const
    {
        return std::make_shared<detail::iteration>(operands_)->loop();
    }
}}}
