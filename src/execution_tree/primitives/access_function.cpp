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
            nullptr, &create_primitive<access_function>,nullptr)
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
        std::vector<primitive_argument_type> const& params,
        eval_mode mode) const
    {
        if (!(mode & eval_dont_wrap_functions) && !params.empty())
        {
            if (!params.empty())
            {
                std::vector<primitive_argument_type> fargs;
                fargs.reserve(params.size() + 1);

                fargs.push_back(extract_ref_value(operands_[0]));
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
                        compiler::compose_primitive_name(name_parts),
                        codename_)
                    });
            }
        }

        return hpx::make_ready_future(extract_ref_value(operands_[0]));
    }

    void access_function::store(std::vector<primitive_argument_type>&& vals,
        std::vector<primitive_argument_type>&& params)
    {
        primitive* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            p->store(hpx::launch::sync, std::move(vals), std::move(params));
        }
    }

    topology access_function::expression_topology(
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

