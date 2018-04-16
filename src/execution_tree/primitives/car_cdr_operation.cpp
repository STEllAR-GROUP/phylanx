// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/car_cdr_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_car_cdr_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("car_cdr");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

#define PHYLANX_CAR_CDR_MATCH_DATA(name)                                       \
    hpx::util::make_tuple(name, std::vector<std::string>{name "(_1)"},         \
        &create_car_cdr_operation, &create_primitive<car_cdr_operation>)       \
/**/

    std::vector<match_pattern_type> const car_cdr_operation::match_data =
    {
        PHYLANX_CAR_CDR_MATCH_DATA("car"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdr"),
        PHYLANX_CAR_CDR_MATCH_DATA("caar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cadr"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cddr"),
        PHYLANX_CAR_CDR_MATCH_DATA("caaar"),
        PHYLANX_CAR_CDR_MATCH_DATA("caadr"),
        PHYLANX_CAR_CDR_MATCH_DATA("cadar"),
        PHYLANX_CAR_CDR_MATCH_DATA("caddr"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdaar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdadr"),
        PHYLANX_CAR_CDR_MATCH_DATA("cddar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdddr"),
        PHYLANX_CAR_CDR_MATCH_DATA("caaaar"),
        PHYLANX_CAR_CDR_MATCH_DATA("caaadr"),
        PHYLANX_CAR_CDR_MATCH_DATA("caadar"),
        PHYLANX_CAR_CDR_MATCH_DATA("caaddr"),
        PHYLANX_CAR_CDR_MATCH_DATA("cadaar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cadadr"),
        PHYLANX_CAR_CDR_MATCH_DATA("caddar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cadddr"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdaaar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdaadr"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdadar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdaddr"),
        PHYLANX_CAR_CDR_MATCH_DATA("cddaar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cddadr"),
        PHYLANX_CAR_CDR_MATCH_DATA("cdddar"),
        PHYLANX_CAR_CDR_MATCH_DATA("cddddr")
    };

#undef PHYLANX_CAR_CDR_MATCH_DATA

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        std::string generate_operation_code(std::string const& name)
        {
            // extract actual name of the primitive and remove leading 'c' and
            // trailing 'r'
            std::string::size_type p = name.find_first_of("$");
            if (p != std::string::npos)
            {
                return name.substr(1, p - 2);
            }
            return name.substr(1, name.size() - 2);
        }
    }

    car_cdr_operation::car_cdr_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , operation_(detail::generate_operation_code(name))
    {
        std::reverse(operation_.begin(), operation_.end());
    }

    primitive_argument_type car_cdr_operation::car(
        primitive_argument_type&& arg) const
    {
        std::vector<primitive_argument_type> list =
            extract_list_value_strict(std::move(arg), name_, codename_);
        if (list.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "car_cdr_operation::car",
                execution_tree::generate_error_message(
                    "the car_cdr_operation primitive requires exactly "
                        "one non-empty list-operand",
                    name_, codename_));
        }
        return list[0];
    }

    primitive_argument_type car_cdr_operation::cdr(
        primitive_argument_type&& arg) const
    {
        std::vector<primitive_argument_type> list =
            extract_list_value_strict(std::move(arg), name_, codename_);
        if (list.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "car_cdr_operation::cdr",
                execution_tree::generate_error_message(
                    "the car_cdr_operation primitive requires exactly "
                        "one non-empty list-operand",
                    name_, codename_));
        }
        list.erase(list.begin());
        return primitive_argument_type{std::move(list)};
    }

    hpx::future<primitive_argument_type> car_cdr_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "car_cdr_operation::eval",
                execution_tree::generate_error_message(
                    "the car_cdr_operation primitive requires exactly "
                        "one (list-) operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "car_cdr_operation::eval",
                execution_tree::generate_error_message(
                    "the car_cdr_operation primitive requires that the "
                        "arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](std::vector<primitive_argument_type>&& list)
            -> primitive_argument_type
            {
                primitive_argument_type result{std::move(list)};

                // handle operation code for the given list
                for (char op : this_->operation_)
                {
                    HPX_ASSERT(op == 'a' || op == 'd');
                    if (op == 'a')
                    {
                        // replace list with first element
                        result = this_->car(std::move(result));
                    }
                    else
                    {
                        // remove first element
                        result = this_->cdr(std::move(result));
                    }
                }

                return result;
            }),
            list_operand(operands_[0], args, name_, codename_));
    }

    // Start iteration over given for statement
    hpx::future<primitive_argument_type> car_cdr_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
