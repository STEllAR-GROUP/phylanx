//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/function.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>
#include <phylanx/execution_tree/primitives/variable_factory.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const variable_factory::match_data =
    {
        hpx::util::make_tuple("variable-factory",
            std::vector<std::string>{},
            nullptr, &create_primitive<variable_factory>,
            "Internal")
    };

    ///////////////////////////////////////////////////////////////////////////
    variable_factory::variable_factory(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
      , create_variable_(compiler::extract_primitive_name(name_) == "variable")
    {
        // operands_[0] is expected to be the actual variable
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable_factory::variable_factory",
                generate_error_message(
                    "the variable_factory primitive requires exactly "
                    "zero operands"));
        }
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> variable_factory::eval(
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // copy operand as we have to move it
        primitive_argument_type body = operands_[0];

        if (create_variable_)
        {
            // create a new instance of the variable, do not register with AGAS
            return hpx::make_ready_future(
                primitive_argument_type{create_variable(hpx::find_here(),
                    std::move(body), name_, codename_, false)});
        }

        // create a new instance of the function, do not register with AGAS
        primitive_argument_type func = create_function(
            hpx::find_here(), std::move(body), name_, codename_, false);

        // if the function should not receive any bound arguments, we're done
        if (!(ctx.mode_ & eval_dont_wrap_functions) && !args.empty())
        {
            // if the function to create has bound arguments, wrap the new
            // function into a target-reference
            primitive_arguments_type fargs;
            fargs.reserve(args.size() + 1);

            fargs.emplace_back(std::move(func));
            for (auto const& arg : args)
            {
                fargs.push_back(extract_value(arg, name_, codename_));
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
        return hpx::make_ready_future(std::move(func));
    }

    void variable_factory::store(primitive_argument_type&& data,
        primitive_arguments_type&& params, eval_context ctx)
    {
        if (valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable_factory::store",
                generate_error_message(
                    "the variable_factory primitive's store method should be "
                    "called only once"));
        }
        operands_[0] = std::move(data);
    }


    ///////////////////////////////////////////////////////////////////////////
    topology variable_factory::expression_topology(
        std::set<std::string>&& functions,
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

