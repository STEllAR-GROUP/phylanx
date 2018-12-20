// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/repeat_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

#include <boost/config.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const repeat_operation::match_data =
    {
        hpx::util::make_tuple("repeat",
        std::vector<std::string>{"repeat(_1,_2)", "repeat(_1,_2,_3)"},
        &create_repeat_operation, &create_primitive<repeat_operation>,
        "a, axis, keepdims\n"
        "Args:\n"
        "\n"
        "    a (vector or matrix) : a scalar, a vector or a matrix\n"
        "    axis (optional, integer): an axis to repeat along. By default, "
        "       flattened input is used.\n"
        "\n"
        "Returns:\n"
        "\n"
        "")
    };

    ///////////////////////////////////////////////////////////////////////////
    repeat_operation::repeat_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename)
    {}

    template <typename T>
    primitive_argument_type repeat_operation::repeat0d0d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        blaze::DynamicVector<T> result(rep.scalar(), arg.scalar());
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat0d1d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep) const
    {
        auto v = rep.vector();
        if (v.size() == 1)
        {
            blaze::DynamicVector<T> result(v[0], arg.scalar());
            return primitive_argument_type{std::move(result)};
        }
        else
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat_operation::repeat0d1d",
                generate_error_message(
                    "the repetition should be a scalar or a unit-size vector"));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeat0d(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep,
        hpx::util::optional<val_type> axis) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat_operation::repeat0d",
                generate_error_message(
                    "the repeat_operation primitive requires operand axis to be "
                    "either 0 or -1 for scalar values."));
        }
        switch (rep.num_dimensions())
        {

        case 0:
            return repeat0d0d(std::move(arg), std::move(rep));

        case 1:
            return repeat0d1d(std::move(arg), std::move(rep));
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "repeat_operation::repeat0d",
            generate_error_message(
                "the repetition should be a scalar or a unit-size vector"));
    }

    template <typename T>
    primitive_argument_type repeat_operation::repeatnd(ir::node_data<T>&& arg,
        ir::node_data<val_type>&& rep,
        hpx::util::optional<val_type> axis) const
    {
        std::size_t a_dims = arg.num_dimensions();
        switch (a_dims)
        {
        case 0:
            return repeat0d(std::move(arg), std::move(rep), axis);

        //case 1:
        //    return repeat1d(std::move(arg), std::move(rep), axis);

        //case 2:
        //    return repeat2d(std::move(arg), std::move(rep), axis);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat_operation::eval",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> repeat_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2 && operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat_operation::eval",
                generate_error_message("the repeat_operation primitive requires "
                                       "exactly one, two, or "
                                       "three operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "repeat_operation::eval",
                    generate_error_message(
                        "the repeat_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_arguments_type&& args)
                ->primitive_argument_type {

            hpx::util::optional<std::int64_t> axis;

            // axis is argument #3
            if (args.size() == 3)
            {
                axis = extract_scalar_integer_value_strict(
                    args[2], this_->name_, this_->codename_);
            }

            switch (extract_common_type(args[0]))
            {
            case node_data_type_bool:
                return this_->repeatnd(extract_boolean_value(std::move(args[0]),
                                           this_->name_, this_->codename_),
                    extract_integer_value_strict(args[1]), axis);
            case node_data_type_int64:
                return this_->repeatnd(extract_integer_value(std::move(args[0]),
                                           this_->name_, this_->codename_),
                    extract_integer_value_strict(args[1]), axis);
            case node_data_type_double:
                return this_->repeatnd(extract_numeric_value(std::move(args[0]),
                                           this_->name_, this_->codename_),
                    extract_integer_value_strict(args[1]), axis);
            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "repeat::eval",
                this_->generate_error_message(
                    "the repeat primitive requires for all arguments "
                    "to be numeric data types"));
        }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
