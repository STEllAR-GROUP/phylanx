// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>
#include <phylanx/plugins/listops/car_cdr_operation.hpp>

#include <phylanx/ir/ranges.hpp>

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
    constexpr char const* const helpstring = R"(
        li
        Args:

            li (list) : a list

        Returns:

        car returns the head of a list, e.g. car([1 ,2, 3]) returns 1
        cdr returns the tail of a list, e.g. cdr([1, 2, 3]) returns [2, 3]
        caar() is the same as car(car())
        cadr() is the same as car(cdr())
        etc.
    )";

    ///////////////////////////////////////////////////////////////////////////
#define PHYLANX_CAR_CDR_MATCH_DATA(name)                                       \
    match_pattern_type{name, std::vector<std::string>{name "(_1)"},            \
        &create_car_cdr_operation, &create_primitive<car_cdr_operation>,       \
        helpstring                                                             \
    }                                                                          \
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
            compiler::primitive_name_parts name_parts;
            if (!compiler::parse_primitive_name(name, name_parts))
            {
                return name.substr(1, name.size() - 2);
            }

            return name_parts.primitive.substr(1, name_parts.primitive.size() - 2);
        }
    }

    car_cdr_operation::car_cdr_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , operation_(detail::generate_operation_code(name))
    {
        std::reverse(operation_.begin(), operation_.end());
    }

    primitive_argument_type car_cdr_operation::car(
        primitive_argument_type&& arg) const
    {
        ir::range list =
            extract_list_value_strict(std::move(arg), name_, codename_);
        if (list.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "car_cdr_operation::car",
                util::generate_error_message(
                    "the car_cdr_operation primitive requires exactly "
                        "one non-empty list-operand",
                    name_, codename_));
        }
        return *list.begin();
    }

    primitive_argument_type car_cdr_operation::cdr(
        primitive_argument_type&& arg) const
    {
        ir::range list =
            extract_list_value_strict(std::move(arg), name_, codename_);
        if (list.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "car_cdr_operation::cdr",
                util::generate_error_message(
                    "the car_cdr_operation primitive requires exactly "
                        "one non-empty list-operand",
                    name_, codename_));
        }

        if (list.is_ref())
        {
            // this list represents a pair of iterators or an integer range
            auto it = list.begin();
            return primitive_argument_type{ir::range{++it, list.end()}};
        }

        // create a copy of the vector excluding index 0
        primitive_arguments_type list_copy;
        list_copy.reserve(list.size() - 1);
        auto it = list.begin();
        std::move(++it, list.end(), std::back_inserter(list_copy));

        return primitive_argument_type{ir::range{std::move(list_copy)}};
    }

    hpx::future<primitive_argument_type> car_cdr_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "car_cdr_operation::eval",
                generate_error_message(
                    "the car_cdr_operation primitive requires exactly "
                        "one (list-) operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "car_cdr_operation::eval",
                generate_error_message(
                    "the car_cdr_operation primitive requires that the "
                        "arguments given by the operands array "
                        "are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](hpx::future<ir::range>&& list)
            -> primitive_argument_type
            {
                primitive_argument_type result{list.get()};

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
            },
            list_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
