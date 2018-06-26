//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/access_function.hpp>

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
    match_pattern_type const access_function::match_data =
    {
        hpx::util::make_tuple("access-function",
            std::vector<std::string>{},
            nullptr, &create_primitive<access_function>)
    };

    ///////////////////////////////////////////////////////////////////////////
    access_function::access_function(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
    {
        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_function::access_function",
                generate_error_message(
                    "the access_function primitive requires at least one "
                        "operand"));
        }

        if (valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> access_function::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        if (!params.empty())
        {
            primitive_argument_type const& target =
                valid(bound_value_) ?
                    bound_value_ :
                    value_operand_sync(operands_[0], params, name_, codename_);

            std::vector<primitive_argument_type> fargs;
            fargs.reserve(params.size() + 1);

            fargs.push_back(extract_value(target));
            for (auto const& param : params)
            {
                fargs.push_back(extract_value(param));
            }

            compiler::primitive_name_parts name_parts =
                compiler::parse_primitive_name(name_);
            name_parts.primitive = "target-reference";

            return hpx::make_ready_future(primitive_argument_type{
                create_primitive_component(hpx::find_here(),
                    name_parts.primitive, std::move(fargs),
                    compiler::compose_primitive_name(name_parts), codename_)
                });
        }

        if (valid(bound_value_))
        {
            return hpx::make_ready_future(bound_value_);
        }
        return value_operand(operands_[0], params, name_, codename_);
    }

    bool access_function::bind(
        std::vector<primitive_argument_type> const& params) const
    {
        bound_value_ = primitive_argument_type{};

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr && p->bind(params))
        {
            bound_value_ = p->eval(hpx::launch::sync, params);
            return true;
        }

        return false;
    }

    void access_function::store(primitive_argument_type&& val)
    {
        bound_value_ = primitive_argument_type{};

        primitive* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            p->store(hpx::launch::sync, std::move(val));
        }
    }

    topology access_function::expression_topology(
        std::set<std::string>&&) const
    {
        return topology{};
    }
}}}

