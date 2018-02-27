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
    struct block_operation::step : std::enable_shared_from_this<step>
    {
        step(std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args,
            std::string const& name, std::string const& codename)
            : operands_(operands)
            , args_(args)
            , name_(name)
            , codename_(codename)
        {
        }

        void next(std::size_t i)
        {
            // skip statements that don't return anything
            while (i != operands_.size() && !valid(operands_[i]))
                ++i;

            if (i == operands_.size())
                return;

            auto this_ = this->shared_from_this();
            value_operand(operands_[i], args_, name_, codename_)
                .then([this_, i](
                            hpx::future<primitive_argument_type>&& step) {
                    try
                    {
                        // the value of the last step is returned
                        if (i == this_->operands_.size() - 1)
                        {
                            this_->result_.set_value(step.get());
                            return;
                        }

                        step.get();            // rethrow exception
                        this_->next(i + 1);    // trigger next step
                    }
                    catch (...)
                    {
                        this_->result_.set_exception(
                            std::current_exception());
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
                    execution_tree::generate_error_message(
                        "the block_operation primitive requires at least "
                        "one argument",
                        name_, codename_));
            }

            next(0);    // trigger first step
            return result_.get_future();
        }

    private:
        std::vector<primitive_argument_type> operands_;
        std::vector<primitive_argument_type> args_;
        hpx::promise<primitive_argument_type> result_;
        std::string name_;
        std::string codename_;
    };

    ///////////////////////////////////////////////////////////////////////////

    primitive create_block_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("block");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const block_operation::match_data =
    {
        hpx::util::make_tuple("block",
            std::vector<std::string>{"block(__1)"},
            &create_block_operation, &create_primitive<block_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////

    block_operation::block_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    // start iteration over given block statement
    hpx::future<primitive_argument_type> block_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<step>(
                args, noargs, name_, codename_)->eval();
        }
        return std::make_shared<step>(
            operands_, args, name_, codename_)->eval();
    }
}}}
