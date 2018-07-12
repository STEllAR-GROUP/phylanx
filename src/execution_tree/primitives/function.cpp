//  Copyright (c) 2017-2018 Hartmut Kaiser
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
        std::string const& codename)
    {
        static std::string type("function");
        return create_primitive_component(
            locality, type, std::move(operand), name, codename);
    }

    match_pattern_type const function::match_data =
    {
        hpx::util::make_tuple("function",
            std::vector<std::string>{},
            nullptr, &create_primitive<function>)
    };

    ///////////////////////////////////////////////////////////////////////////
    function::function(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
      , num_arguments_(std::size_t(-1))
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
            operands_[0] = extract_copy_value(std::move(operands_[0]));
        }
    }

    hpx::future<primitive_argument_type> function::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        return hpx::make_ready_future(extract_ref_value(operands_[0]));
    }

    bool function::bind(
        std::vector<primitive_argument_type> const& args,
        bind_mode mode) const
    {
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "function::bind",
                generate_error_message(
                    "the expression representing the function target "
                        "has not been initialized"));
        }

        // return if the bound function expects more arguments than provided
        if (num_arguments_ != std::size_t(-1) && args.size() < num_arguments_)
        {
            return false;
        }

        // evaluation of the define-function yields the function body
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->bind(args, mode);
        }
        return true;
    }

    void function::store(primitive_argument_type&& data)
    {
        operands_[0] = extract_copy_value(std::move(data));
    }

    void function::set_num_arguments(std::size_t num_args)
    {
        num_arguments_ = num_args;
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

