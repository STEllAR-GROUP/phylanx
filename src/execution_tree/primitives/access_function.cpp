//  Copyright (c) 2017-2019 Hartmut Kaiser
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
            nullptr, &create_primitive<access_function>,
            "Internal")
    };

    ///////////////////////////////////////////////////////////////////////////
    access_function::access_function(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
      , target_name_(compiler::extract_instance_name(name_))
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
            operands_[0] =
                extract_copy_value(std::move(operands_[0]), name_, codename_);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> access_function::eval(
        primitive_arguments_type const& params, eval_context ctx) const
    {
        // access variable from execution context
        auto const* target = ctx.get_var(target_name_);
        if (target == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_function::eval",
                generate_error_message(
                    hpx::util::format("the variable '{}' is unbound in the "
                                      "current execution environment",
                        target_name_)));
        }

        if (!(ctx.mode_ & eval_dont_wrap_functions) && !params.empty())
        {
            primitive_arguments_type fargs;
            fargs.reserve(params.size() + 1);

            fargs.push_back(extract_ref_value(*target, name_, codename_));
            for (auto const& param : params)
            {
                fargs.push_back(extract_value(param, name_, codename_));
            }

            compiler::primitive_name_parts name_parts =
                compiler::parse_primitive_name(name_);
            name_parts.primitive = "target-reference";

            return hpx::make_ready_future(primitive_argument_type{
                create_primitive_component(hpx::find_here(),
                    name_parts.primitive, std::move(fargs), std::move(ctx),
                    compiler::compose_primitive_name(name_parts),
                    codename_)
                });
        }

        return hpx::make_ready_future(
            extract_ref_value(*target, name_, codename_));
    }

    void access_function::store(primitive_arguments_type&& vals,
        primitive_arguments_type&& params, eval_context ctx)
    {
        // access variable from execution context
        auto* target = ctx.get_var(target_name_);
        if (target == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_function::store",
                generate_error_message(
                    hpx::util::format("the variable '{}' is unbound in the "
                                      "current execution environment",
                        target_name_)));
        }

        primitive* p = util::get_if<primitive>(target);
        if (p != nullptr)
        {
            p->store(hpx::launch::sync, std::move(vals), std::move(params),
                std::move(ctx));
        }
    }

    void access_function::store(primitive_argument_type&& val,
        primitive_arguments_type&& params, eval_context ctx)
    {
        // access variable from execution context
        auto* target = ctx.get_var(target_name_);
        if (target == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_function::store",
                generate_error_message(
                    hpx::util::format("the variable '{}' is unbound in the "
                                      "current execution environment",
                        target_name_)));
        }

        primitive* p = util::get_if<primitive>(target);
        if (p != nullptr)
        {
            p->store(hpx::launch::sync, std::move(val), std::move(params),
                std::move(ctx));
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

