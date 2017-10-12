// Copyright (c) 2017 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/for_operation.hpp>
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
    phylanx::execution_tree::primitives::for_operation>
    for_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    for_operation_type, phylanx_for_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(for_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const for_operation::match_data =
    {
        "for(_1, _2, _3)", &create<for_operation>
    };

    ///////////////////////////////////////////////////////////////////////////
    for_operation::for_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::for_operation::"
                    "for_operation",
                "the for_operation primitive requires exactly three arguments");
        }

        if (!valid(operands_[0]) || !valid(operands_[1]) || !valid(operands_[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::for_operation::"
                    "for_operation",
                "the for_operation primitive requires that the arguments "
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

            hpx::future<primitive_result_type> init()
            {
                auto this_ = this->shared_from_this();
                return literal_operand(operands_[0]).then(
                //do eval for initialization
                );
            }

            hpx::future<primitive_result_type> body(
                hpx::future<primitive_result_type>&& cond)
            {
                if (extract_boolean_value(cond.get()))
                {
                    // evaluate body of for statement
                    auto this_ = this->shared_from_this();
                    return literal_operand(operands_[2]).then(
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
                // evaluate condition of for statement
                auto this_ = this->shared_from_this();
                return literal_operand(operands_[1]).then(
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

    // start iteration over given for statement
    hpx::future<primitive_result_type> for_operation::eval() const
    {
        return std::make_shared<detail::iteration>(operands_)->loop();
    }
}}}
