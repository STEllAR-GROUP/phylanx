//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/access_variable.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>

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
    match_pattern_type const access_variable::match_data =
    {
        hpx::util::make_tuple("access-variable",
            std::vector<std::string>{},
            nullptr, &create_primitive<access_variable>)
    };

    ///////////////////////////////////////////////////////////////////////////
    access_variable::access_variable(
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename, true)
    {
        // operands_[0] is expected to be the actual variable, operands_[1] and
        // operands_[2] are optional slicing arguments

        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_variable::access_variable",
                generate_error_message(
                    "the access_variable primitive requires at least one "
                    "operand"));
        }
        if (operands_.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_variable::access_variable",
                generate_error_message(
                    "the access_variable primitive requires at most three "
                    "operands"));
        }

        if (valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));

            // FIXME: allow for the slicing parameters to be a result of
            //        evaluating expressions
            if (operands_.size() > 1)
            {
                if (!is_list_operand_strict(operands_[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "access_variable::access_variable",
                        generate_error_message(
                            "the row-slicing construct associated with the "
                            "variable must be represented as a range"));
                }
            }

            if (operands_.size() > 2)
            {
                if (!is_list_operand_strict(operands_[2]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "access_variable::access_variable",
                        generate_error_message(
                            "the column-slicing construct associated with the "
                            "variable must be represented as a range"));
                }
            }
        }

        // try to bind to the variable object locally
        primitive* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            hpx::error_code ec(hpx::lightweight);
            target_ = hpx::get_ptr<primitive_component>(
                hpx::launch::sync, p->get_id(), ec);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> access_variable::eval(
        std::vector<primitive_argument_type> const& params,
        eval_mode mode) const
    {
        // handle slicing, we can replace the params with our slicing
        // parameter as variable evaluation can't depend on those anyways
        if (operands_.size() == 2 || operands_.size() == 3)
        {
            std::vector<primitive_argument_type> args;
            for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
            {
                args.emplace_back(extract_ref_value(*it));
            }

            if (target_)
            {
                return target_->eval(std::move(args),
                    eval_mode(mode | eval_dont_wrap_functions));
            }

            return value_operand(operands_[0], std::move(args), name_,
                codename_, eval_mode(mode | eval_dont_wrap_functions));
        }

        // no slicing parameters given, access variable directly
        if (target_)
        {
            return target_->eval(
                noargs, eval_mode(mode | eval_dont_wrap_functions));
        }

        return value_operand(operands_[0], noargs, name_, codename_,
            eval_mode(mode | eval_dont_wrap_functions));
    }

    void access_variable::store(std::vector<primitive_argument_type>&& vals)
    {
        if (vals.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_variable::store",
                generate_error_message(
                    "invoking store with slicing parameters is not supported"));
        }

        // handle slicing, simply append the slicing parameters to the end of
        // the argument list
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            vals.emplace_back(extract_ref_value(*it));
        }

        if (target_)
        {
            target_->store(std::move(vals));
        }
        else
        {
            primitive* p = util::get_if<primitive>(&operands_[0]);
            if (p != nullptr)
            {
                p->store(hpx::launch::sync, std::move(vals));
            }
        }
    }

    topology access_variable::expression_topology(
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

