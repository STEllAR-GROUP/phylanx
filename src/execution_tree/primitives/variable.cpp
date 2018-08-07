//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/slice.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>
#include <phylanx/ir/ranges.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/unlock_guard.hpp>

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
    primitive create_variable(hpx::id_type const& locality,
        primitive_argument_type&& operand, std::string const& name,
        std::string const& codename)
    {
        static std::string type("variable");
        return create_primitive_component(
            locality, type, std::move(operand), name, codename);
    }

    match_pattern_type const variable::match_data =
    {
        hpx::util::make_tuple("variable",
            std::vector<std::string>{},
            nullptr, &create_primitive<variable>)
    };

    ///////////////////////////////////////////////////////////////////////////
    variable::variable(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
      , value_set_(false)
    {
        // operands_[0] is expected to be the actual variable
        if (operands_.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable::variable",
                generate_error_message(
                    "the variable primitive requires no more than three "
                    "operands"));
        }

        if (!operands_.empty())
        {
            // the first argument is the expression the variable should be
            // bound to
            operands_[0] = extract_copy_value(std::move(operands_[0]));
            value_set_ = true;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> variable::eval(
        std::vector<primitive_argument_type> const& args, eval_mode mode) const
    {
        if (!value_set_ && !valid(bound_value_))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::eval",
                generate_error_message(
                    "the expression representing the variable target "
                    "has not been initialized"));
        }

        primitive_argument_type const& target =
            valid(bound_value_) ? bound_value_ : operands_[0];

        // if given, args[0] and args[1] are optional slicing arguments
        if (!args.empty() && !(mode & eval_dont_evaluate_partials))
        {
            if (args.size() > 1)
            {
                // handle row/column-slicing
                return hpx::make_ready_future(slice(target, args[0], args[1]));
            }

            // handle row/tuple-slicing
            return hpx::make_ready_future(slice(target, args[0]));
        }

        return hpx::make_ready_future(extract_ref_value(target));
    }

    hpx::future<primitive_argument_type> variable::eval(
        primitive_argument_type && arg, eval_mode mode) const
    {
        if (!value_set_ && !valid(bound_value_))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::eval",
                generate_error_message(
                    "the expression representing the variable target "
                    "has not been initialized"));
        }

        primitive_argument_type const& target =
            valid(bound_value_) ? bound_value_ : operands_[0];

        // if given, args[0] and args[1] are optional slicing arguments
        if (valid(arg) && !(mode & eval_dont_evaluate_partials))
        {
            // handle row/tuple-slicing
            return hpx::make_ready_future(slice(target, arg));
        }

        return hpx::make_ready_future(extract_ref_value(target));
    }

    //////////////////////////////////////////////////////////////////////////
    bool variable::bind(std::vector<primitive_argument_type> const& args) const
    {
        if (!value_set_)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::bind",
                generate_error_message(
                    "the expression representing the variable target "
                        "has not been initialized"));
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            bound_value_ = extract_copy_value(p->eval(hpx::launch::sync, args));
        }
        else
        {
            bound_value_ = extract_ref_value(operands_[0]);
        }

        return true;
    }

    void variable::store(std::vector<primitive_argument_type>&& data,
        std::vector<primitive_argument_type>&& params)
    {
        // data[0] is the new value to store in this variable
        // data[1] and data[2] (optional) are interpreted as slicing arguments
        if (data.empty())
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::bind",
                generate_error_message(
                    "the right hand side expression is not valid"));
        }

        if (!value_set_ || !valid(operands_[0]))
        {
            if (data.size() > 1)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "variable::bind",
                    generate_error_message(
                        "the initial expression a variable is bound to is "
                        "not allowed to have slicing parameters"));
            }

            operands_[0] = extract_copy_value(std::move(data[0]));
            value_set_ = true;
        }
        else
        {
            switch (data.size())
            {
            case 1:
                bound_value_ = extract_copy_value(std::move(data[0]));
                return;

            case 2:
                {
                    auto result = slice(std::move(bound_value_),
                        value_operand_sync(
                            data[1], std::move(params), name_, codename_),
                        std::move(data[0]));
                    bound_value_ = std::move(result);
                    return;
                }

            case 3:
                {
                    auto data1 =
                        value_operand_sync(data[1], params, name_, codename_);
                    auto result = slice(std::move(bound_value_), data1,
                        value_operand_sync(
                            data[2], std::move(params), name_, codename_),
                        std::move(data[0]));
                    bound_value_ = std::move(result);
                    return;
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::bind",
                generate_error_message(
                    "there can be at most two slicing arguments"));
        }
    }

    void variable::store(primitive_argument_type&& data,
        std::vector<primitive_argument_type>&& params)
    {
        // data is the new value to store in this variable
        if (!value_set_ || !valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(data));
            value_set_ = true;
        }
        else
        {
            bound_value_ = extract_copy_value(std::move(data));
        }
    }

    topology variable::expression_topology(std::set<std::string>&& functions,
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

