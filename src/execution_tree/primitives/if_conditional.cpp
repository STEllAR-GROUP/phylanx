//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Adrian Serio
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/if_conditional.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_if_conditional(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("if");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const if_conditional::match_data =
    {
        hpx::util::make_tuple("if",
            std::vector<std::string>{"if(_1, _2, _3)", "if(_1, _2)"},
            &create_if_conditional, &create_primitive<if_conditional>)
    };

    ///////////////////////////////////////////////////////////////////////////
    if_conditional::if_conditional(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    // struct containing implementation of evaluate
    // (used to resolve lifetime issues)
    namespace detail
    {
        struct if_impl : std::enable_shared_from_this<if_impl>
        {
            if_impl(std::vector<primitive_argument_type> const& operands,
                    std::vector<primitive_argument_type> const& args,
                    std::string const& name, std::string const& codename)
              : operands_(operands)
              , args_(args)
              , name_(name)
              , codename_(codename)
            {
                if (operands_.size() != 3 && operands_.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "if_conditional::if_conditional",
                        generate_error_message(
                            "the if_conditional primitive requires three "
                                "operands",
                            name_, codename_));
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
                        generate_error_message(
                            "the if_conditional primitive requires that the "
                                "arguments given by the operands array "
                                "is valid",
                            name_, codename_));
                }
            }

            hpx::future<primitive_argument_type> body()
            {
                // Keep data alive with a shared pointer
                auto this_ = this->shared_from_this();
                return boolean_operand(operands_[0], args_, name_, codename_)
                    .then([this_](hpx::future<std::uint8_t> && cond_eval)
                        -> hpx::future<primitive_argument_type>
                    {
                        if (cond_eval.get() != 0)
                        {
                            return literal_operand(this_->operands_[1],
                                this_->args_, this_->name_, this_->codename_);
                        }
                        if (this_->operands_.size() > 2)
                        {
                            return literal_operand(this_->operands_[2],
                                this_->args_, this_->name_, this_->codename_);
                        }
                        return hpx::make_ready_future(primitive_argument_type{});
                    });
            }

        private:
            std::vector<primitive_argument_type> operands_;
            std::vector<primitive_argument_type> args_;
            std::string name_;
            std::string codename_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // evaluate 'true_case' or 'false_case' based on 'cond'
    hpx::future<primitive_argument_type> if_conditional::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::if_impl>(
                args, noargs, name_, codename_)->body();
        }
        return std::make_shared<detail::if_impl>(
            operands_, args, name_, codename_)->body();
    }
}}}
