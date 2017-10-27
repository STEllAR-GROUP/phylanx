//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/block_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/ast.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::block_operation>
    block_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    block_operation_type, phylanx_block_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(block_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const block_operation::match_data =
    {
        hpx::util::make_tuple("block", "block(__1)", &create<block_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    block_operation::block_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {}

    namespace detail
    {
        struct step : std::enable_shared_from_this<step>
        {
            step(std::vector<primitive_argument_type> const& operands,
                    std::vector<primitive_argument_type> const& args)
              : operands_(operands)
              , args_(args)
            {}

            void next(std::size_t i)
            {
                // skip statements that don't return anything
                while (i != operands_.size() && !valid(operands_[i]))
                    ++i;

                if (i == operands_.size())
                    return;

                auto this_ = this->shared_from_this();
                literal_operand(operands_[i], args_).then(
                    [this_, i](hpx::future<primitive_result_type> && step)
                    {
                        try
                        {
                            // the value of the last step is returned
                            if (i == this_->operands_.size() - 1)
                            {
                                this_->result_.set_value(step.get());
                                return;
                            }

                            step.get();             // rethrow exception
                            this_->next(i + 1);     // trigger next step
                        }
                        catch (...)
                        {
                            this_->result_.set_exception(std::current_exception());
                        }
                    });
            }

            hpx::future<primitive_result_type> eval()
            {
                if (operands_.empty())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "block_operation::eval",
                        "the block_operation primitive requires at least one "
                            "argument");
                }

                next(0);            // trigger first step
                return result_.get_future();
            }

        private:
            std::vector<primitive_argument_type> operands_;
            std::vector<primitive_argument_type> args_;
            hpx::promise<primitive_result_type> result_;
        };
    }

    // start iteration over given block statement
    hpx::future<primitive_result_type> block_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            static std::vector<primitive_argument_type> noargs;
            return std::make_shared<detail::step>(args, noargs)->eval();
        }

        return std::make_shared<detail::step>(operands_, args)->eval();
    }
}}}
