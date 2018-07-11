//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/access_variable.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const access_variable::match_data =
    {
        hpx::util::make_tuple("access-variable",
            std::vector<std::string>{},
            nullptr, &create_primitive<access_variable>)
    };

    ///////////////////////////////////////////////////////////////////////////
    access_variable::access_variable(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
    {
        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_variable::access_variable",
                generate_error_message(
                    "the access_variable primitive requires at least one "
                        "operand"));
        }

        if (valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> access_variable::eval(
        std::vector<primitive_argument_type> const& params,
        eval_mode mode) const
    {
        if (valid(bound_value_))
        {
            return hpx::make_ready_future(bound_value_);
        }
        return value_operand(operands_[0], params, name_, codename_,
            eval_mode(mode | eval_dont_wrap_functions));
    }

    bool access_variable::bind(
        std::vector<primitive_argument_type> const& params) const
    {
        bound_value_ = primitive_argument_type{};

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr && p->bind(params))
        {
            bound_value_ =
                extract_copy_value(p->eval(hpx::launch::sync, params));
            return true;
        }

        return false;
    }

    void access_variable::store(primitive_argument_type&& val)
    {
        bound_value_ = primitive_argument_type{};

        primitive* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            p->store(hpx::launch::sync, std::move(val));
        }
    }

    topology access_variable::expression_topology(
        std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            std::string name = p->registered_name();
            if (resolve_children.find(name) != resolve_children.end())
            {
                // recurse into function, if asked to do that
                return p->expression_topology(hpx::launch::sync,
                    std::move(functions), std::move(resolve_children));
            }

            // add only the name of the direct dependent node (no recursion)
            return topology{std::move(name)};
        }
        return {};
    }
}}}

