//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/wrapped_function.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const wrapped_function::match_data =
    {
        hpx::util::make_tuple("call-function",
            std::vector<std::string>{},
            nullptr, &create_primitive<wrapped_function>)
    };

    ///////////////////////////////////////////////////////////////////////////
    std::string extract_function_name(std::string const& name)
    {
        if (name.find("define-") == 0)
        {
            return name.substr(7);
        }
        return name;
    }

    wrapped_function::wrapped_function(
            std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {
        // the first entry of operands represents the target
        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "wrapped_function::wrapped_function",
                execution_tree::generate_error_message(
                    "no target given",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> wrapped_function::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        // evaluation of the define-function yields the function body
        primitive_argument_type body;

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            body = value_operand_sync(operands_[0], params, name_, codename_);
        }
        else
        {
            body = operands_[0];
        }

        std::vector<primitive_argument_type> fargs;
        if (operands_.size() == 1)
        {
            // no pre-bound arguments
            fargs.reserve(params.size() + 1);
            fargs.push_back(std::move(body));
            std::copy(params.begin(), params.end(), std::back_inserter(fargs));

            static std::string type("function");

            return hpx::make_ready_future(
                primitive_argument_type{
                    create_primitive_component(hpx::find_here(),
                        type, std::move(fargs),
                        extract_function_name(name_), codename_)
                });
        }

        fargs.reserve(operands_.size() + params.size() - 1);
        std::copy(params.begin(), params.end(), std::back_inserter(fargs));
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            fargs.push_back(value_operand_sync(*it, params, name_, codename_));
        }
        return value_operand(body, std::move(fargs), name_, codename_);
    }

    primitive_argument_type wrapped_function::bind(
        std::vector<primitive_argument_type> const& params) const
    {
        // evaluation of the define-function yields the function body
        primitive_argument_type body;

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            body = p->bind(params);
        }
        else
        {
            body = operands_[0];
        }

        std::vector<primitive_argument_type> fargs;
        if (operands_.size() == 1)
        {
            // no pre-bound arguments
            fargs.reserve(params.size() + 1);
            fargs.push_back(std::move(body));
            std::copy(params.begin(), params.end(), std::back_inserter(fargs));

            static std::string type("function");

            return primitive_argument_type{
                    create_primitive_component(hpx::find_here(),
                        type, std::move(fargs),
                        extract_function_name(name_), codename_)
                };
        }

        fargs.reserve(operands_.size() - 1);
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            fargs.push_back(value_operand_sync(*it, params, name_, codename_));
        }
        return value_operand_sync(body, std::move(fargs), name_, codename_);
    }

    topology wrapped_function::expression_topology(
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

