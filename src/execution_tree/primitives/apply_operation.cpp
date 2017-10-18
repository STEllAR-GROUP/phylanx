//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_identifier.hpp>
#include <phylanx/ast/detail/is_placeholder_ellipses.hpp>
#include <phylanx/execution_tree/generate_tree.hpp>
#include <phylanx/execution_tree/primitives/apply_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/eigen.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::apply_operation>
    apply_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    apply_operation_type, phylanx_apply_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(apply_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const apply_operation::match_data =
    {
        hpx::util::make_tuple(
            "apply", "apply(_1, __2)", &create<apply_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    apply_operation::apply_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        std::size_t size = operands_.size();
        if (size < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "apply_operation::apply_operation",
                "the apply function requires at least two operands");
        }

        using function_type = std::vector<ast::expression>;

        function_type* func = util::get_if<function_type>(&operands_[0]);
        if (func == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "apply_operation::apply_operation",
                "the first argument to the apply function has to be a "
                    "function");
        }

        using list_type = phylanx::util::recursive_wrapper<
            std::vector<primitive_argument_type>>;

        list_type* list = util::get_if<list_type>(&operands_[size - 1]);
        if (list == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "apply_operation::apply_operation",
                "the last argument to the apply function has to be a list");
        }

        // verify number of arguments
        std::size_t numargs = size - 2 + list->get().size();
        std::size_t numparams = func->size() - 2;

        if (numargs != numparams)
        {
            // look at last formal argument
            if (!ast::detail::is_identifier((*func)[numparams+1]))
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "apply_operation::apply_operation",
                    "the formal parameter does not represent a valid variable "
                        "name");
            }

            // last format argument is a placeholder_ellipses
            std::string lastargname =
                ast::detail::identifier_name((*func)[numparams+1]);
            if (ast::detail::is_placeholder_ellipses(lastargname))
            {
                if (numparams < numargs)
                {
                    HPX_THROW_EXCEPTION(hpx::invalid_status,
                        "apply_operation::apply_operation",
                        "the number of actual arguments does not match the "
                            "number of expected formal parameters");
                }
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "apply_operation::apply_operation",
                    "the number of actual arguments does not match the "
                        "number of expected formal parameters");
            }
        }

        // generate function object
        phylanx::execution_tree::variables variables;

        auto const& patterns = phylanx::execution_tree::get_all_known_patterns();
        phylanx::execution_tree::functions functions(
            phylanx::execution_tree::detail::builtin_functions(patterns));

        // replace formal arguments (all except last argument, which is a list
        // and requires special handling)
        for (std::size_t i = 1; i != size - 1; ++i)
        {
            using value_type =
                typename phylanx::execution_tree::variables::value_type;

            if (!ast::detail::is_identifier((*func)[i]))
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "apply_operation::apply_operation",
                    "the formal parameter does not represent a valid variable "
                        "name");
            }

            std::string argname = ast::detail::identifier_name((*func)[i]);
            auto pvar = variables.find(argname);
            if (is_empty_range(pvar))
            {
                variables.insert(value_type(std::move(argname), operands_[i]));
                continue;
            }

            if (ast::detail::is_placeholder_ellipses(argname))
            {
                // this is the last available formal argument (all parameters
                // coming after that have to be handled in the same way as the
                // list
                for (/**/; i != size - 1; ++i)
                {
                    list_type* pv = util::get_if<list_type>(&pvar.first->second);
                    if (pv != nullptr)
                    {
                        pv->get().push_back(operands_[i]);
                    }
                    else
                    {
                        *pv = std::vector<primitive_argument_type>{
                            std::move(pvar.first->second), operands_[i]
                        };
                    }
                }
                break;
            }
            else
            {
                pvar.first->second = operands_[i];
            }
        }

        // handle last argument


        // instantiate the new function
        auto result = execution_tree::detail::generate_tree((*func).back(),
            execution_tree::detail::generate_patterns(patterns),
            variables, functions, hpx::find_here());
    }

    // implement 'apply'
    hpx::future<primitive_result_type> apply_operation::eval() const
    {
        primitive p;
        return p.eval();
    }
}}}
