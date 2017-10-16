//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_identifier.hpp>
#include <phylanx/execution_tree/generate_tree.hpp>
#include <phylanx/execution_tree/primitives/define.hpp>

#include <hpx/include/util.hpp>

#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const define_::match_data =
    {
        // We don't need a creation function as 'define()' is explicitly
        // handled by generate_tree.
        hpx::util::make_tuple("define", "define(__1)", nullptr)
    };

    ///////////////////////////////////////////////////////////////////////////
    primitive create_function_invocation(hpx::id_type where,
        std::vector<primitive_argument_type>&& args, variables& vars,
        functions& funcs)
    {
        // args[0] represents the function name the remaining elements in args
        // refer to the parameters (variables) or literal values to use while
        // instantiating the function.

        std::string* name = util::get_if<std::string>(&args[0]);
        if (name == nullptr)
        {
            if (args.size() != 1 || !is_primitive_operand(args[0]))
            {
                // function name is invalid
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::execution_tree::primitives::"
                        "create_function_invocation",
                    "the given function name is invalid");
            }
            return primitive_operand(args[0]);
        }

        auto p = funcs.find(*name);
        if (is_empty_range(p))
        {
            // function was not defined
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::execution_tree::primitives::"
                    "create_function_invocation",
                "the function to invoke was not defined: " + *name);
        }

        auto it = p.first;
        if (it->second.first.size() != args.size() - 1)
        {
            // argument number mismatch
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "create_function_invocation",
                "the number of given arguments does not match the expected "
                    "number of formal parameters for function: " + *name);
        }

        // augment the symbol and function tables
        phylanx::execution_tree::variables variables(&vars);
        phylanx::execution_tree::functions functions(&funcs);

        // replace formal arguments
        for (std::size_t i = 0; i != args.size() - 1; ++i)
        {
            using value_type =
                typename phylanx::execution_tree::variables::value_type;

            if (!ast::detail::is_identifier(it->second.first[i]))
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::execution_tree::primitives::"
                        "create_function_invocation",
                    "the formal parameter does not represent a valid variable "
                        "name");
            }

            std::string argname =
                ast::detail::identifier_name(it->second.first[i]);

            auto pvar = variables.find(argname);
            if (is_empty_range(pvar))
            {
                variables.insert(
                    value_type(std::move(argname), std::move(args[i + 1])));
            }
            else
            {
                pvar.first->second = std::move(args[i + 1]);
            }
        }

        // instantiate the new function
        auto result = execution_tree::detail::generate_tree(it->second.second,
            execution_tree::detail::generate_patterns(get_all_known_patterns()),
            variables, functions);

        return primitive_operand(result);
    }

    std::vector<match_pattern_type> const define_::invocation_match_data =
    {
        hpx::util::make_tuple(
            "any_function", "_1(__2)", &create_function_invocation)
    };
}}}
