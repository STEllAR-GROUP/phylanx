//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/define_variable.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const define_variable::match_data =
    {
        hpx::util::make_tuple("define-variable",
            std::vector<std::string>{},
            nullptr, &create_primitive<define_variable>,
            "Internal")
    };

    match_pattern_type const define_variable::match_data_define =
    {
        hpx::util::make_tuple("define",
            std::vector<std::string>{"define(__1)"},
            nullptr, nullptr,
            R"(name,args,body
            Args:

                name (string) : name of symbol to define
                args (optional,list of symbols) : if  present,
                                     this defines a function.
                body (expression) : value to bind to symbol `name`
                                  or body of lambda function.

            Returns:)"
            )
    };

    match_pattern_type const define_variable::match_data_lambda =
    {
        hpx::util::make_tuple("lambda",
            std::vector<std::string>{"lambda(__1)"},
            nullptr, nullptr,
            R"(args,body
            Args:

                *args (argument list): the list of arguments
                body (statemt): the body of the lambda function

            Returns:

            A function object with the arguments and body specified.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    define_variable::define_variable(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
        // body is assumed to be operands_[0]
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "define_variable::define_variable",
                generate_error_message("the define_variable primitive requires "
                    "exactly one operand"));
        }
    }

    hpx::future<primitive_argument_type> define_variable::eval(
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // evaluate the expression bound to this name and store the value in
        // the associated variable
        primitive* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
             p->bind(args, std::move(ctx));
        }
        return hpx::make_ready_future(
            extract_ref_value(operands_[0], name_, codename_));
    }

    void define_variable::store(primitive_arguments_type&& vals,
        primitive_arguments_type&& params)
    {
        if (!valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_variable::store",
                generate_error_message(
                    "the variable associated with this define has not been "
                        "initialized yet"));
        }
        if (vals.empty())
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_variable::store",
                generate_error_message(
                    "the right hand side expression is not valid"));
        }

        primitive* p = util::get_if<primitive>(&operands_[1]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_variable::store",
                generate_error_message(
                    "the variable associated with this define has not been "
                        "properly initialized"));
        }

        p->store(hpx::launch::sync, std::move(vals[0]), std::move(params));
    }

    topology define_variable::expression_topology(
        std::set<std::string>&& functions,
        std::set<std::string>&& resolve_children) const
    {
        if (functions.find(name_) != functions.end())
        {
            return {};      // avoid recursion
        }

        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_variable::expression_topology",
                util::generate_error_message(
                    "expression represented by the variable was not "
                        "initialized yet",
                    name_, codename_));
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
