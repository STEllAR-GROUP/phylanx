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
            nullptr, &create_primitive<define_variable>)
    };

    match_pattern_type const define_variable::match_data_define =
    {
        hpx::util::make_tuple("define",
            std::vector<std::string>{"define(__1)"},
            nullptr, nullptr)
    };

    ///////////////////////////////////////////////////////////////////////////
    define_variable::define_variable(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
        // body is assumed to be operands_[0]
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "define_variable::define_variable",
                execution_tree::generate_error_message(
                    "the define_variable primitive requires exactly one "
                        "operand",
                    name_, codename_));
        }

        // target is assumed to be operands_[1]
        operands_.resize(2);
    }

    hpx::future<primitive_argument_type> define_variable::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        // this proxy was created where the variable should be created on.
        if (!valid(operands_[1]))
        {
            std::vector<primitive_argument_type> operands;
            operands.push_back(operands_[0]);

            auto name_parts = compiler::parse_primitive_name(name_);

            HPX_ASSERT(name_parts.primitive == "define-variable");
            name_parts.primitive = "variable";

            operands_[1] = primitive_argument_type{
                create_primitive_component(
                    hpx::find_here(), name_parts.primitive, std::move(operands),
                    compiler::compose_primitive_name(name_parts), codename_)
                };

            // bind this name to the result of the expression right away
            primitive* p = util::get_if<primitive>(&operands_[1]);
            if (p != nullptr)
            {
                p->eval(hpx::launch::sync, args);
            }

            return hpx::make_ready_future(extract_ref_value(operands_[1]));
        }

        // just evaluate the expression bound to this name
        return value_operand(operands_[1], args, name_, codename_);
    }

    void define_variable::store(primitive_argument_type && val)
    {
        if (!valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_variable::store",
                execution_tree::generate_error_message(
                    "the variable associated with this define has not been "
                        "initialized yet",
                    name_, codename_));
        }

        primitive* p = util::get_if<primitive>(&operands_[1]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_variable::store",
                execution_tree::generate_error_message(
                    "the variable associated with this define has not been "
                        "properly initialized",
                    name_, codename_));
        }
        p->store(hpx::launch::sync, std::move(val));
    }

    topology define_variable::expression_topology(
        std::set<std::string>&& functions) const
    {
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_variable::expression_topology",
                execution_tree::generate_error_message(
                    "expression represented by the variable was not "
                        "initialized yet",
                    name_, codename_));
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->expression_topology(
                hpx::launch::sync, std::move(functions));
        }
        return {};
    }
}}}
