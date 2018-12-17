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
            nullptr, &create_primitive<variable>,
            "Internal"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    variable::variable(primitive_arguments_type&& operands,
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
            operands_[0] =
                extract_copy_value(std::move(operands_[0]), name_, codename_);
            value_set_ = true;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> variable::eval(
        primitive_arguments_type const& args, eval_context ctx) const
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

        // if given, args[0], args[1] and args[2] are optional slicing arguments
        if (!args.empty() && !(ctx.mode_ & eval_dont_evaluate_partials))
        {
            if (args.size() == 2)
            {
                // handle row/column-slicing
                return hpx::make_ready_future(
                    slice(target, args[0], args[1], name_, codename_));
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            if (args.size() > 2)
            {
                // handle page/row/column-slicing
                return hpx::make_ready_future(
                    slice(target, args[0], args[1], args[2], name_, codename_));
            }
#endif
            // handle row-slicing
            return hpx::make_ready_future(
                slice(target, args[0], name_, codename_));
        }

        return hpx::make_ready_future(
            extract_ref_value(target, name_, codename_));
    }

    hpx::future<primitive_argument_type> variable::eval(
        primitive_argument_type && arg, eval_context ctx) const
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

        // if given, args[0] is an optional slicing argument
        if (valid(arg) && !(ctx.mode_ & eval_dont_evaluate_partials))
        {
            // handle row-slicing
            return hpx::make_ready_future(
                slice(target, std::move(arg), name_, codename_));
        }

        return hpx::make_ready_future(
            extract_ref_value(target, name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    bool variable::bind(primitive_arguments_type const& args,
        eval_context ctx) const
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
            bound_value_ = extract_copy_value(
                p->eval(hpx::launch::sync, args, std::move(ctx)),
                name_, codename_);
        }
        else
        {
            bound_value_ = extract_ref_value(operands_[0], name_, codename_);
        }

        return true;
    }

    void variable::store1dslice(primitive_arguments_type&& data,
        primitive_arguments_type&& params)
    {
        if (!valid(bound_value_))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::store1dslice",
                generate_error_message(
                    "in order for slicing to be possible a variable must have "
                    "a value bound to it"));
        }

        auto result = slice(std::move(bound_value_),
            value_operand_sync(
                std::move(data[1]), std::move(params), name_, codename_),
            std::move(data[0]), name_, codename_);
        bound_value_ = std::move(result);
    }

    void variable::store2dslice(primitive_arguments_type&& data,
        primitive_arguments_type&& params)
    {
        if (!valid(bound_value_))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::store2dslice",
                generate_error_message(
                    "in order for slicing to be possible a variable must have "
                    "a value bound to it"));
        }

        auto data1 =
            value_operand_sync(data[1], params, name_, codename_);
        auto result = slice(std::move(bound_value_), std::move(data1),
            value_operand_sync(
                data[2], std::move(params), name_, codename_),
            std::move(data[0]), name_, codename_);
        bound_value_ = std::move(result);
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    void variable::store3dslice(primitive_arguments_type&& data,
        primitive_arguments_type&& params)
    {
        if (!valid(bound_value_))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::store3dslice",
                generate_error_message(
                    "in order for slicing to be possible a variable must have "
                    "a value bound to it"));
        }

        auto data1 = value_operand_sync(data[1], params, name_, codename_);
        auto data2 = value_operand_sync(data[2], params, name_, codename_);
        auto result = slice(
            std::move(bound_value_), std::move(data1), std::move(data2),
            value_operand_sync(data[3], std::move(params), name_, codename_),
            std::move(data[0]), name_, codename_);
        bound_value_ = std::move(result);
    }
#endif

    void variable::store(primitive_arguments_type&& data,
        primitive_arguments_type&& params)
    {
        // data[0] is the new value to store in this variable
        // data[1] and optionally data[2]/data[3] are interpreted as slicing
        // arguments
        if (data.empty())
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::store",
                generate_error_message(
                    "the right hand side expression is not valid"));
        }

        if (!value_set_ || !valid(operands_[0]))
        {
            if (data.size() > 1)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "variable::store",
                    generate_error_message(
                        "the initial expression a variable is bound to is "
                        "not allowed to have slicing parameters"));
            }

            operands_[0] =
                extract_copy_value(std::move(data[0]), name_, codename_);
            value_set_ = true;
        }
        else
        {
            switch (data.size())
            {
            case 1:
                bound_value_ =
                    extract_copy_value(std::move(data[0]), name_, codename_);
                return;

            case 2:
                store1dslice(std::move(data), std::move(params));
                return;

            case 3:
                store2dslice(std::move(data), std::move(params));
                return;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            case 4:
                store3dslice(std::move(data), std::move(params));
                return;
#endif

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::store",
                generate_error_message(
                    "there can be at most two slicing arguments"));
        }
    }

    void variable::store(primitive_argument_type&& data,
        primitive_arguments_type&& params)
    {
        // data is the new value to store in this variable
        if (!value_set_ || !valid(operands_[0]))
        {
            operands_[0] =
                extract_copy_value(std::move(data), name_, codename_);
            value_set_ = true;
        }
        else
        {
            bound_value_ =
                extract_copy_value(std::move(data), name_, codename_);
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

