//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Adrian Serio
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/if_conditional.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/eigen.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::if_conditional>
    if_conditional_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(if_conditional_type,
    phylanx_if_conditional_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(if_conditional_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx {
namespace execution_tree {
    namespace primitives {
        ///////////////////////////////////////////////////////////////////////////
        std::vector<match_pattern_type> const if_conditional::match_data = {
            hpx::util::make_tuple(
                "if", "if(_1, _2, _3)", &create<if_conditional>),
            hpx::util::make_tuple("if", "if(_1, _2)", &create<if_conditional>)};

        ///////////////////////////////////////////////////////////////////////////
        if_conditional::if_conditional(
            std::vector<primitive_argument_type>&& operands)
          : operands_(std::move(operands))
        {
            if (operands_.size() != 3 && operands_.size() != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "if_conditional::if_conditional",
                    "the if_conditional primitive requires three operands");
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
                    "if_conditional::if_conditional",
                    "the if_conditional primitive requires that the arguments "
                    "given "
                    "by the operands array is valid");
            }
        }

        ///////////////////////////////////////////////////////////////////////////
        // struct containing implementation of evaluate
        // (used to resolve lifetime issues)
        namespace detail {
            struct if_impl : std::enable_shared_from_this<if_impl>
            {
                if_impl(std::vector<primitive_argument_type> const& operands)
                  : operands_(operands)
                {
                }

                hpx::future<primitive_result_type> body()
                {
                    // Keep data alive with a shared pointer
                    auto this_ = this->shared_from_this();
                    hpx::future<std::uint8_t> cond_eval =
                        boolean_operand(operands_[0]);
                    return cond_eval.then(
                        [this_](hpx::future<std::uint8_t>&& cond_eval) {
                            if (cond_eval.get() != 0)
                            {
                                return literal_operand(this_->operands_[1]);
                            }
                            else if (this_->operands_.size() > 2)
                            {
                                return literal_operand(this_->operands_[2]);
                            }
                            else
                            {
                                return hpx::make_ready_future(
                                    primitive_result_type{});
                            }
                        });
                }

            private:
                std::vector<primitive_argument_type> operands_;
            };
        }
        ///////////////////////////////////////////////////////////////////////////
        // evaluate 'true_case' or 'false_case' based on 'cond'
        hpx::future<primitive_result_type> if_conditional::eval() const
        {
            return std::make_shared<detail::if_impl>(operands_)->body();
        }
    }
}
}
