//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/lambda.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const lambda::match_data =
    {
        hpx::util::make_tuple("lambda",
            std::vector<std::string>{"lambda(__1)"},
            nullptr, &create_primitive<lambda>)
    };

    ///////////////////////////////////////////////////////////////////////////
    lambda::lambda(std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
      , num_arguments_(0)
    {
        // the first entry of operands represents the target
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "lambda::lambda",
                generate_error_message(
                    "the lambda primitive needs exactly one argument"));
        }

        if (valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> lambda::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "lambda::eval",
                generate_error_message(
                    "the expression representing the function target "
                        "has not been initialized"));
        }

        // simply invoke the given body with the given arguments
        return value_operand(
            operands_[0], args, name_, codename_, eval_dont_wrap_functions);
    }

    bool lambda::bind(std::vector<primitive_argument_type> const& params) const
    {
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "lambda::bind",
                generate_error_message(
                    "the expression representing the function target "
                        "has not been initialized"));
        }

        // return if the bound function expects more arguments than provided
//         if (params.size() < num_arguments_)
//         {
//             return false;
//         }

        // evaluation of the define-function yields the function body
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->bind(params);
        }
        return true;
    }

    void lambda::store(primitive_argument_type&& data)
    {
        if (valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "lambda::store",
                generate_error_message(
                    "the expression representing the function target "
                        "has already been initialized"));
        }

        // initialize the lambda's body
        operands_[0] = extract_copy_value(std::move(data));
    }

    void lambda::set_num_arguments(std::size_t num_args)
    {
        num_arguments_ = num_args;
    }

    topology lambda::expression_topology(
        std::set<std::string>&& functions) const
    {
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->expression_topology(
                hpx::launch::sync, std::move(functions));
        }
        return {};
    }
}}}

