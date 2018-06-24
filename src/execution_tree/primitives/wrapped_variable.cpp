//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/wrapped_variable.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const wrapped_variable::match_data =
    {
        hpx::util::make_tuple("access-variable",
            std::vector<std::string>{},
            nullptr, &create_primitive<wrapped_variable>)
    };

    ///////////////////////////////////////////////////////////////////////////
    wrapped_variable::wrapped_variable(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
    {
        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "wrapped_variable::wrapped_variable",
                execution_tree::generate_error_message(
                    "the wrapped_variable primitive requires at least one "
                        "operand",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> wrapped_variable::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        std::vector<primitive_argument_type> fargs;
        fargs.reserve(operands_.size() - 1);

        // pass along pre-bound arguments
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            fargs.push_back(value_operand_sync(*it, params, name_, codename_));
        }

        return value_operand(operands_[0], std::move(fargs), name_, codename_);
    }

    primitive_argument_type wrapped_variable::bind(
        std::vector<primitive_argument_type> const& params) const
    {
        // evaluation of the define-function yields the function body
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            std::vector<primitive_argument_type> fargs;
            fargs.reserve(operands_.size() - 1);

            for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
            {
                // bind pre-bound arguments using actual ones
                primitive const* arg = util::get_if<primitive>(&*it);
                if (arg != nullptr)
                {
                    fargs.push_back(arg->bind(params));
                }
                else
                {
                    fargs.push_back(extract_ref_value(*it));
                }
            }

            p->bind(std::move(fargs));
        }
        return {};
    }

    void wrapped_variable::store(primitive_argument_type && val)
    {
        primitive* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            p->store(hpx::launch::sync, std::move(val));
        }
    }

    topology wrapped_variable::expression_topology(
        std::set<std::string>&&) const
    {
        return topology{};
    }
}}}

