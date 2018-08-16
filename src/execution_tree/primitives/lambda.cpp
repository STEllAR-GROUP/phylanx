//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/lambda.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/util/assert.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const lambda::match_data =
    {
        hpx::util::make_tuple("lambda",
            std::vector<std::string>{"lambda(__1)"},
            nullptr, &create_primitive<lambda>,
            "synopsis: lambda(body)\n"
            "synopsis: lambda(x0, body)\n"
            "synopsis: lambda(x0, x1, ..., body)\n"
            "The last argument `body` is the body of the function, the preceeding "
            "arguments (`x0`, `x1`, etc.) are all arguments to the lambda function. "
            "TODODOC"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    lambda::lambda(std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {
        // the first entry of operands represents the target
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "lambda::lambda",
                generate_error_message(
                    "the lambda primitive needs exactly one argument"));
        }

        if (valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> lambda::eval(
        std::vector<primitive_argument_type> const& args, eval_mode mode) const
    {
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "lambda::eval",
                generate_error_message(
                    "the expression representing the function target "
                        "has not been initialized"));
        }

        if (mode & eval_dont_evaluate_lambdas)
        {
            if (!args.empty())
            {
                std::vector<primitive_argument_type> fargs;
                fargs.reserve(args.size() + 1);

                fargs.push_back(extract_ref_value(operands_[0]));
                for (auto const& arg : args)
                {
                    fargs.push_back(extract_value(arg));
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

            return hpx::make_ready_future(extract_ref_value(operands_[0]));
        }

        // simply invoke the given body with the given arguments
        return value_operand(operands_[0], args, name_, codename_,
            eval_mode(eval_dont_evaluate_lambdas | eval_dont_wrap_functions));
    }

    void lambda::store(std::vector<primitive_argument_type>&& data,
        std::vector<primitive_argument_type>&& params)
    {
        if (valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "lambda::store",
                generate_error_message(
                    "the expression representing the function target "
                        "has already been initialized"));
        }
        if (data.empty())
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "lambda::store",
                generate_error_message(
                    "the right hand side expression is not valid"));
        }
        if (!params.empty())
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "lambda::store",
                generate_error_message(
                    "store shouldn't be called with dynamic arguments"));
        }

        // initialize the lambda's body
        operands_[0] = extract_copy_value(std::move(data[0]));
    }

    topology lambda::expression_topology(std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->expression_topology(hpx::launch::sync,
                std::move(functions), std::move(resolve_children));
        }
        return {};
    }
}}}

