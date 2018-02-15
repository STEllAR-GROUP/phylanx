//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/block_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_block_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("block");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const block_operation::match_data =
    {
        hpx::util::make_tuple("block",
            std::vector<std::string>{"block(__1)"},
            &create_block_operation, &create_primitive<block_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    block_operation::block_operation(
            std::vector<primitive_argument_type>&& operands)
      : primitive_component_base(std::move(operands))
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
                value_operand(operands_[i], args_).then(
                    [this_, i](hpx::future<primitive_argument_type> && step)
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

            hpx::future<primitive_argument_type> eval()
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
            hpx::promise<primitive_argument_type> result_;
        };
    }

    // start iteration over given block statement
    hpx::future<primitive_argument_type> block_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::step>(args, noargs)->eval();
        }

        return std::make_shared<detail::step>(operands_, args)->eval();
    }
}}}
