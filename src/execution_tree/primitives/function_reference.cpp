//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/function_reference.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const function_reference::match_data =
    {
        hpx::util::make_tuple("function",
            std::vector<std::string>{},
            nullptr, &create_primitive<function_reference>)
    };

    ///////////////////////////////////////////////////////////////////////////
    function_reference::function_reference(
            std::vector<primitive_argument_type>&& args, std::string const& name,
            std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {
        // operands_[0] holds the target function
        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "function_reference::function_reference",
                execution_tree::generate_error_message(
                    "no target given",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> function_reference::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        if (operands_.size() > 1)
        {
            // the function has prebound arguments
            std::vector<primitive_argument_type> fargs(
                operands_.begin() + 1, operands_.end());

            if (!params.empty())
            {
                fargs.reserve(operands_.size() + params.size() - 1);
                std::copy(params.begin(), params.end(), std::back_inserter(fargs));
                return value_operand(
                    operands_[0], std::move(fargs), name_, codename_);
            }

            return value_operand(
                operands_[0], std::move(fargs), name_, codename_);
        }

        return value_operand(operands_[0], params, name_, codename_);
    }

    topology function_reference::expression_topology(
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

