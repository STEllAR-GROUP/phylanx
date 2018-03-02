// Copyright (c) 2017 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/for_operation.hpp>
#include <phylanx/ir/node_data.hpp>

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
    primitive create_for_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("for");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const for_operation::match_data =
    {
        hpx::util::make_tuple("for",
            std::vector<std::string>{"for(_1, _2, _3, _4)"},
            &create_for_operation, &create_primitive<for_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    for_operation::for_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    struct for_operation::iteration_for
      : std::enable_shared_from_this<iteration_for>
    {
        iteration_for(std::shared_ptr<for_operation const> that)
            : that_(that)
        {
            if (that_->operands_.size() != 4)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::for_operation::"
                        "eval",
                    execution_tree::generate_error_message(
                        "the for_operation primitive requires exactly "
                            "four arguments",
                        that->name_, that->codename_));
            }

            if (!valid(that_->operands_[0]) || !valid(that_->operands_[1]) ||
                !valid(that_->operands_[2]) || !valid(that_->operands_[3]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::for_operation::"
                        "eval",
                    execution_tree::generate_error_message(
                        "the for_operation primitive requires that the "
                            "arguments given by the operands array are "
                            "valid",
                        that_->name_, that_->codename_));
            }
        }

        hpx::future<primitive_argument_type> init(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args)
        {
            this->args_ = args;
            auto this_ = this->shared_from_this();
            return value_operand(
                that_->operands_[0], args_, that_->name_, that_->codename_)
                .then([this_](hpx::future<primitive_argument_type>&& val)
                          -> hpx::future<primitive_argument_type> {
                    val.get();
                    return this_->loop();
                });
        }

        hpx::future<primitive_argument_type> loop()
        {
            // Evaluate condition of for statement
            auto this_ = this->shared_from_this();
            return value_operand(
                that_->operands_[1], args_, that_->name_, that_->codename_)
                .then([this_](hpx::future<primitive_argument_type>&& cond)
                          -> hpx::future<primitive_argument_type>
                {
                    return this_->body(std::move(cond));
                });
        }

        hpx::future<primitive_argument_type> body(
            hpx::future<primitive_argument_type>&& cond)
        {
            if (extract_boolean_value(cond.get(), that_->name_, that_->codename_))
            {
                // Evaluate body of for statement
                auto this_ = this->shared_from_this();
                return value_operand(
                    that_->operands_[3], args_, that_->name_, that_->codename_)
                    .then([this_](hpx::future<primitive_argument_type>&&
                                  result) mutable
                        -> hpx::future<primitive_argument_type> {
                        this_->result_ = result.get();
                        return this_->reinit();    // Do the reinit statement
                    });
            }

            return hpx::make_ready_future(result_);
        }

        hpx::future<primitive_argument_type> reinit()
        {
            auto this_ = this->shared_from_this();
            return value_operand(
                that_->operands_[2], args_, that_->name_, that_->codename_)
                .then([this_](hpx::future<primitive_argument_type>&& val)
                          -> hpx::future<primitive_argument_type> {
                    val.get();
                    return this_->loop();   // Call the loop again
                });
        }

    private:
        std::vector<primitive_argument_type> args_;
        primitive_argument_type result_;
        std::shared_ptr<for_operation const> that_;
    };

    // Start iteration over given for statement
    hpx::future<primitive_argument_type> for_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<iteration_for>(
                shared_from_this())->init(args, noargs);
        }
        return std::make_shared<iteration_for>(
            shared_from_this())->init(operands_, args);
    }
}}}
