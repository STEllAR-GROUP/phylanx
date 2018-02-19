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
            nullptr, &create_primitive_with_name<wrapped_function>)
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
            std::vector<primitive_argument_type>&& args, std::string name)
      : primitive_component_base(std::move(args))
      , name_(extract_function_name(name))
    {
        // the first entry of operands represents the target
        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "wrapped_function::wrapped_function",
                "no target given");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> wrapped_function::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                " wrapped_function::eval", "no target given");
        }

        // evaluation of the define-function yields the function body
        auto body = p->eval_direct(params);

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
                    create_primitive_component_with_name(hpx::find_here(),
                        type, std::move(fargs), name_)
                });
        }

        fargs.reserve(operands_.size() - 1);
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            fargs.push_back(value_operand_sync(*it, params));
        }
        return value_operand(body, std::move(fargs));
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

