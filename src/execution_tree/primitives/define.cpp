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
        std::vector<primitive_argument_type>&& args, variables const& vars,
        functions const& funcs)
    {
        // args[0] represents the function name the remaining elements in args
        // refer to the parameters (variables) or literal values to use while
        // instantiating the function.

        std::string* name = util::get_if<std::string>(&args[0]);
        if (name == nullptr)
        {
            // function name is invalid
        }

        auto it = funcs.find(*name);
        if (it != funcs.end())
        {
            // function was not defined
        }

        if (it->second.first.size() != args.size() - 1)
        {
            // argument number mismatch
        }

        // create a copy of the symbol and function tables
        phylanx::execution_tree::variables variables(vars);
        phylanx::execution_tree::functions functions(funcs);

        // replace formal arguments
        for (std::size_t i = 0; i != args.size() - 1 ; ++i)
        {
            using value_type =
                typename phylanx::execution_tree::variables::value_type;

            if (!ast::detail::is_identifier(it->second.first[i]))
            {
            }

            std::string argname =
                ast::detail::identifier_name(it->second.first[i]);

            auto varit = variables.find(argname);
            if (varit == variables.end())
            {
                variables.insert(
                    value_type(std::move(argname), std::move(args[i + 1])));
            }
            else
            {
                varit->second = std::move(args[i + 1]);
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
