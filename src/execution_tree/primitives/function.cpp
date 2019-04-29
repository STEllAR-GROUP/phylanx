//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/function.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/unlock_guard.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_function(hpx::id_type const& locality,
        primitive_argument_type&& operand, std::string const& name,
        std::string const& codename, bool register_with_agas)
    {
        static std::string type("function");
        return create_primitive_component(locality, type, std::move(operand),
            name, codename, register_with_agas);
    }

    match_pattern_type const function::match_data =
    {
        hpx::util::make_tuple("function",
            std::vector<std::string>{},
            nullptr, &create_primitive<function>,
            "Internal"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    function::function(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
      , value_set_(false)
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "function::function",
                generate_error_message(
                    "the function primitive requires exactly one operand"));
        }

        if (valid(operands_[0]))
        {
            operands_[0] =
                extract_copy_value(std::move(operands_[0]), name_, codename_);
            value_set_ = true;
        }
    }

    hpx::future<primitive_argument_type> function::eval(
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (!value_set_)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "function::eval",
                generate_error_message(
                    "the expression representing the function target "
                        "has not been initialized"));
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->eval(args, std::move(ctx));
        }
        return hpx::make_ready_future(
            extract_value(operands_[0], name_, codename_));
    }

    bool function::bind(
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (!value_set_)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "function::bind",
                generate_error_message(
                    "the expression representing the function target "
                        "has not been initialized"));
        }
        return true;
    }

    void function::store(primitive_argument_type&& data,
        primitive_arguments_type&& params, eval_context ctx)
    {
        if (!valid(data))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "function::store",
                generate_error_message(
                    "the right hand side expression is not valid"));
        }
        if (!params.empty())
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "function::store",
                generate_error_message(
                    "store shouldn't be called with dynamic arguments"));
        }

        operands_[0] = extract_copy_value(std::move(data), name_, codename_);
        value_set_ = true;
    }

    topology function::expression_topology(std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        if (functions.find(name_) != functions.end())
        {
            return {};      // avoid recursion
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            functions.insert(name_);
            return p->expression_topology(hpx::launch::sync,
                std::move(functions), std::move(resolve_children));
        }
        return {};
    }
}}}

